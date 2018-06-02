#include <thermite.hpp>
#include <thermite/voice_client.hpp>

#include "internal.hpp"
#include "udp_internal.hpp"

#include <sodium.h>
#include <type_traits>

using namespace thermite::discord;

inline void sendUdp(
    uv_udp_t* handle,
    struct sockaddr* addr,
    detail::renter&& rent)
{
    // must stay alive for duration of uv_udp_send
    auto* req = new detail::udpSendData{std::move(rent)};
    uv_udp_send(req->uv(), handle, req->bufs(), req->num_bufs(), addr,
        [](uv_udp_send_t* req, int status)
        {
            (void)status;
            delete static_cast<detail::udpSendData*>(req->data);
        });
}

template <typename T,
    int N = sizeof(T) * 8,
    typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
inline T swapEndianness(T value)
{
    T result = 0;

    for (int i = 0; i < N; i += 8)
    {
        result |= ((value >> i) & 0xFF) << (N - i - 8);
    }

    return result;
}


template <typename T,
    typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
inline void writeIntoPacket(
    std::vector<unsigned char>& packet,
    size_t pos,
    T value)
{
    T* ptr = reinterpret_cast<T*>(packet.data() + pos);
    // TODO: only swap if needed
    ptr[0] = swapEndianness(value);
}

void voice_client::send_opus_packet(
    const std::vector<unsigned char>& frame,
    uint32_t frame_samples)
{
    detail::renter buffer{
        12 + // header size
        frame.size() + // message length
        crypto_secretbox_MACBYTES}; // plus mac bytes for ciphertext length
    auto& packet = buffer.buffer();

    packet[0] = 0x80; packet[1] = 0x78;
    writeIntoPacket(packet, 2, _sequence);
    writeIntoPacket(packet, 4, _timestamp);
    writeIntoPacket(packet, 8, _ssrc);

    _sequence++;
    _timestamp += frame_samples;

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    memset(nonce, 0, crypto_secretbox_NONCEBYTES);
    size_t nonce_append_bytes = 0;

    switch (_mode)
    {
        case voice_mode::XSalsa20_Poly1305_Lite:
            randombytes_buf(nonce, 4);
            nonce_append_bytes = 4;
            break;
        case voice_mode::XSalsa20_Poly1305_Suffix:
            randombytes_buf(nonce, crypto_secretbox_NONCEBYTES);
            nonce_append_bytes = crypto_secretbox_NONCEBYTES;
            break;
        case voice_mode::XSalsa20_Poly1305:
            // No data to append since it's purely the header
            memcpy(nonce, packet.data(), 12);
            nonce_append_bytes = 0;
            break;
        case voice_mode::Unknown:
            break;
    }

    crypto_secretbox_easy(
        // Offset for header
        packet.data() + 12,
        frame.data(),
        frame.size(),
        nonce,
        _secret.data());

    // Append the nonce at the end of the packet if it is needed
    if (nonce_append_bytes > 0)
    {
        size_t initial = packet.size();
        packet.resize(initial + nonce_append_bytes);
        memcpy(packet.data() + initial, nonce, nonce_append_bytes);
    }

    sendUdp(&_socket,
        reinterpret_cast<struct sockaddr*>(&_sendAddr),
        std::move(buffer));
}

void voice_client::sendDiscovery()
{
    DEBUG_LOG("Sending discovery");

    detail::renter buffer{70};
    auto& packet = buffer.buffer();

    writeIntoPacket(packet, 0, _ssrc);

    sendUdp(&_socket,
            reinterpret_cast<struct sockaddr*>(&_sendAddr),
            std::move(buffer));
}

void detail::onSocketData(
    uv_udp_t* handle,
    ssize_t read,
    const uv_buf_t* buf,
    const struct sockaddr*,
    unsigned)
{
    DEBUG_LOG("UDP client received data");
    auto* client = static_cast<voice_client*>(handle->data);

    if (read == 70)
    {
        char* data = buf->base + 4;
        std::string ip{data};
        data += ip.size();
        while (*data == '\0')
            data += 1;
        unsigned short port = *(reinterpret_cast<unsigned short*>(data));

        client->finishNegotiateEncryptionMode(ip, port);
    }

    // NOTE: see below for why we do this
    delete[] buf->base;
    client->stopUdpReceive();
}

static void mallocReceiveBuffer(
    uv_handle_t*,
    size_t suggested_size,
    uv_buf_t* buf);

void voice_client::beginUdpReceive()
{
    uv_udp_recv_start(&_socket, mallocReceiveBuffer, detail::onSocketData);
}

void voice_client::stopUdpReceive()
{
    uv_udp_recv_stop(&_socket);
}

/*
 * NOTE: since we only read once per connecting client, I'm okay with
 * allocating 70 bytes for it to be deleted later after we're done.
 *
 * If this becomes a problem in the future, we should consider moving to a more
 * appropriate system. (Maybe rewrite the above rent/return queue above to use
 * a std::pair<size_t, char*>?)
 */
static void mallocReceiveBuffer(
    uv_handle_t*,
    size_t suggested_size,
    uv_buf_t* buf)
{
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}

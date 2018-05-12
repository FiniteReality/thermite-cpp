#include <thermite.hpp>
#include <thermite/voice_client.hpp>

#include "internal.hpp"

#include <sodium.h>
#include <tbb/concurrent_queue.h>
#include <vector>

using namespace thermite::discord;

tbb::concurrent_queue<std::vector<unsigned char>> buffers;

static std::vector<unsigned char> rentBuffer(size_t size)
{
    std::vector<unsigned char> buffer;
    buffers.try_pop(buffer);

    if (buffer.size() < size)
        buffer.resize(size);

    return buffer;
}

static void returnBuffer(std::vector<unsigned char>&& buffer)
{
    buffer.clear();
    buffers.push(std::move(buffer));
}

class renter
{
    public:
        renter(size_t size)
            : _buffer{rentBuffer(size)}, _valid{true}
        { }

        renter(const renter& other) = delete;
        renter& operator=(const renter& other) = delete;

        renter(renter&& other)
            : _buffer{std::move(other._buffer)}, _valid{true}
        {
            other._valid = false;
        }

        renter& operator=(renter&& other)
        {
            if (this != &other)
            {
                other._valid = false;
                _buffer = std::move(other._buffer);
                _valid = true;
            }

            return *this;
        }

        ~renter()
        {
            if (_valid)
            {
                returnBuffer(std::move(_buffer));
            }
        }

        std::vector<unsigned char> useBuffer(std::vector<unsigned char>&& buffer)
        {
            auto old_buffer = _buffer;
            _buffer = std::move(buffer);
            _valid = true;
            return old_buffer;
        }

        std::vector<unsigned char>& buffer()
        {
            return _buffer;
        }

    private:
        std::vector<unsigned char> _buffer;
        bool _valid;
};

class udpSendData
{
    public:
        udpSendData(renter&& rent)
        : _rent{std::move(rent)}, _buf{}, _send{}
        {
            char* uv_data = reinterpret_cast<char*>(_rent.buffer().data());
            _buf = uv_buf_init(uv_data, _rent.buffer().size());
            _send.data = this;
        }

        size_t num_bufs()
        {
            return 1;
        }

        uv_buf_t* buf()
        {
            return &_buf;
        }

        uv_udp_send_t* uv()
        {
            return &_send;
        }

    private:
        renter _rent;
        uv_buf_t _buf;
        uv_udp_send_t _send;
};

static inline void sendUdp(
    uv_udp_t* handle,
    struct sockaddr* addr,
    renter&& rent)
{
    // must stay alive for duration of uv_udp_send
    auto* req = new udpSendData{std::move(rent)};
    uv_udp_send(req->uv(), handle, req->buf(), req->num_bufs(), addr,
        [](uv_udp_send_t* req, int status)
        {
            (void)status;
            delete static_cast<udpSendData*>(req->data);
        });
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
    delete buf->base;
    uv_udp_recv_stop(handle);
}

template <class T>
inline void writeIntoPacket(
    std::vector<unsigned char>& packet,
    size_t pos,
    T value)
{
    T* ptr = reinterpret_cast<T*>(packet.data() + pos);
    ptr[0] = value;
    // HACK: this does not support big-endian systems!
    std::reverse(packet.data() + pos, packet.data() + pos + sizeof(value));
}

/*
#define crypto_secretbox_xsalsa20poly1305_KEYBYTES 32U

#define crypto_secretbox_xsalsa20poly1305_NONCEBYTES 24U

#define crypto_secretbox_xsalsa20poly1305_MACBYTES 16U
*/

void voice_client::sendOpusFrame(
    const std::vector<unsigned char>& frame,
    int frame_ms)
{
    DEBUG_LOG("Sending opus frame");
    renter buffer{
        12 + // header size
        frame.size() + // ciphertext
        crypto_secretbox_MACBYTES}; // mac (from crypto_secretbox)
    auto& packet = buffer.buffer();
    packet[0] = 0x80; packet[1] = 0x78;

    writeIntoPacket(packet, 2, _sequence);
    writeIntoPacket(packet, 4, _timestamp);
    writeIntoPacket(packet, 8, _ssrc);

    _sequence++;
    _timestamp += frame_ms;

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    memset(nonce, 0, crypto_secretbox_NONCEBYTES);
    size_t nonce_size = 0;

    switch (_mode)
    {
        case voice_mode::XSalsa20_Poly1305_Lite:
            randombytes_buf(nonce, 4);
            nonce_size = 4;
            break;
        case voice_mode::XSalsa20_Poly1305_Suffix:
            randombytes_buf(nonce, crypto_secretbox_NONCEBYTES);
            nonce_size = crypto_secretbox_NONCEBYTES;
            break;
        case voice_mode::XSalsa20_Poly1305:
            memcpy(nonce, packet.data(), 12);
            nonce_size = 0;
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

    if (nonce_size > 0)
    {
        size_t initial = packet.size();
        packet.resize(packet.size() + nonce_size);
        memcpy(packet.data() + initial, nonce, nonce_size);
    }

    sendUdp(&_socket,
        reinterpret_cast<struct sockaddr*>(&_sendAddr),
        std::move(buffer));
}

/*
 * NOTE: since we only read once per connecting client, I'm okay with
 * allocating 70 bytes for it to be deleted later after we're done.
 *
 * If this becomes a problem in the future, we should consider moving to a more
 * appropriate system. (Maybe rewrite the above rent/return queue above to use
 * a std::pair<size_t, char*>?)
 */
static void mallocBuffer(
    uv_handle_t*,
    size_t suggested_size,
    uv_buf_t* buf)
{
    char* data = new char[suggested_size];
    buf->base = data;
    buf->len = suggested_size;
}

void voice_client::sendDiscovery()
{
    DEBUG_LOG("Sending discovery");

    renter buffer{70};
    auto& packet = buffer.buffer();

    writeIntoPacket(packet, 0, _ssrc);

    sendUdp(&_socket,
        reinterpret_cast<struct sockaddr*>(&_sendAddr),
        std::move(buffer));

    uv_udp_recv_start(&_socket, mallocBuffer, &detail::onSocketData);
}

void voice_client::sendKeepalive()
{
    DEBUG_LOG("Sending keepalive");
    renter buffer{8};
    auto& packet = buffer.buffer();

    writeIntoPacket(packet, 0, _lastKeepalive++);

    sendUdp(&_socket,
        reinterpret_cast<struct sockaddr*>(&_sendAddr),
        std::move(buffer));
}

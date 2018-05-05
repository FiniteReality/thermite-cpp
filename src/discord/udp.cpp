#include <thermite.hpp>
#include <thermite/voice_client.hpp>

#include "internal.hpp"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <memory>

using namespace thermite::discord;

static void mallocBuffer(
    uv_handle_t*,
    size_t suggested_size,
    uv_buf_t* buf)
{
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}

// TODO: find a better way to do this with less allocations
static inline void sendUdp(
    uv_udp_t* handle,
    struct sockaddr* addr,
    char* data,
    size_t size)
{
    char* local_data = new char[size];
    memcpy(local_data, data, size);
    uv_buf_t buf = uv_buf_init(local_data, size);

    uv_udp_send_t *req = new uv_udp_send_t;
    req->data = local_data;
    uv_udp_send(req, handle, &buf, 1, addr,
        [](uv_udp_send_t* req, int status)
        {
            (void)status;
            delete[] static_cast<char*>(req->data);
            delete req;
        });
}

void detail::onSocketData(
    uv_udp_t* handle,
    ssize_t read,
    const uv_buf_t* buf,
    const struct sockaddr*,
    unsigned)
{
    auto* client = static_cast<voice_client*>(handle->data);

    if (read == 70)
    {
        char* data = buf->base + 4;
        std::string ip{data};
        data += ip.size() + 1;
        unsigned short port = *(reinterpret_cast<unsigned short*>(data));

        client->finishNegotiateEncryptionMode(ip, port);
    }

    delete buf->base;
    uv_udp_recv_stop(handle);
}

void voice_client::sendDiscovery()
{
    char discovery[70];
    int* _discovery = reinterpret_cast<int*>(discovery);
    _discovery[0] = _ssrc;

    sendUdp(&_socket,
        reinterpret_cast<struct sockaddr*>(&_send_addr),
        discovery, 70);

    uv_udp_recv_start(&_socket, mallocBuffer, detail::onSocketData);
}

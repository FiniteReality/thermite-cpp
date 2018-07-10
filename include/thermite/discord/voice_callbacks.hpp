#ifndef VOICE_CALLBACKS_HPP
#define VOICE_CALLBACKS_HPP

#include <uv.h>
#include <uWS/uWS.h>

namespace thermite
{
namespace discord
{
namespace detail
{

void onWSMessage(
    uWS::WebSocket<false>* wsClient,
    const char* message,
    size_t size,
    uWS::OpCode opcode);

void onWSConnect(
    uWS::WebSocket<false>* wsClient,
    uWS::HttpRequest);

void onWSDisconnect(
    uWS::WebSocket<false>* wsClient,
    int code,
    const char* message,
    size_t length);

void onSocketData(
    uv_udp_t* handle,
    ssize_t read,
    const uv_buf_t* buf,
    const struct sockaddr* addr,
    unsigned flags);

}
}
}

#endif /* VOICE_CALLBACKS_HPP */

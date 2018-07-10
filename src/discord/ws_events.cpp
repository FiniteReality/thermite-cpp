#include <thermite/discord/voice_client.hpp>

#include <uWS/uWS.h>

using namespace thermite::discord;

void detail::onWSMessage(
    uWS::WebSocket<false>* wsClient,
    const char* message,
    size_t size,
    uWS::OpCode opcode)
{
    auto client = static_cast<voice_client*>(
        wsClient->getUserData());

    client->onWSMessage(message, size, opcode);
}

void detail::onWSConnect(
    uWS::WebSocket<false>* wsClient,
    uWS::HttpRequest)
{
    auto client = static_cast<voice_client*>(
        wsClient->getUserData());

    client->onWSConnect(wsClient);
}

void detail::onWSDisconnect(
    uWS::WebSocket<false>* wsClient,
    int code,
    const char* msg,
    size_t length)
{
    auto client = static_cast<voice_client*>(
        wsClient->getUserData());
    std::string message{msg, length};

    client->onWSDisconnect(code, message);
}

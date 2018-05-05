#include <thermite.hpp>
#include "discord/internal.hpp"

#include <uWS/uWS.h>

using namespace thermite::discord;

#ifdef THERMITE_DEBUG

std::ostream& thermite::discord::operator<<(
    std::ostream& lhs,
    const voice_opcode& rhs)
{
    return lhs << static_cast<int>(rhs);
}

#endif /* THERMITE_DEBUG */

void detail::onWSMessage(
    uWS::WebSocket<false>* wsClient,
    const char* message,
    size_t size,
    uWS::OpCode opcode)
{
    auto client = static_cast<thermite::discord::voice_client*>(
        wsClient->getUserData());

    client->onWSMessage(message, size, opcode);
}

void detail::onWSConnect(
    uWS::WebSocket<false>* wsClient,
    uWS::HttpRequest)
{
    auto client = static_cast<thermite::discord::voice_client*>(
        wsClient->getUserData());

    client->_websocket = wsClient;
}

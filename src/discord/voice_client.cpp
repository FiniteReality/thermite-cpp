#include <fstream>

#include <thermite.hpp>
#include <thermite/voice_client.hpp>

#include "internal.hpp"

#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

using namespace thermite::discord;

void voice_client::connect(
    std::string server_id,
    std::string user_id,
    std::string endpoint,
    std::string session,
    std::string token)
{
    _listen = true;
    _nonce = 0;
    _last_received_nonce = 0;

    _server_id = server_id;
    _user_id = user_id;
    _session = session;
    _token = token;

    auto url = getVersionedUri(endpoint);
    DEBUG_LOG("connecting to: " << url);
    detail::connect(url, this);
}

void voice_client::disconnect()
{
    uv_timer_stop(&_heartbeat_timer);
    _websocket->close();
    _listen = false;
}

void voice_client::play_file(std::string location)
{ }


void voice_client::sendWSMessage(rapidjson::Document& document)
{
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer{buf};
    document.Accept(writer);

    _websocket->send(buf.GetString(), buf.GetSize(), uWS::OpCode::TEXT);
}

void voice_client::onWSMessage(
    const char* data,
    size_t count,
    uWS::OpCode opcode)
{
    switch (opcode)
    {
        case uWS::OpCode::TEXT:
        {
            rapidjson::Document d;
            d.Parse(data, count);
            rapidjson::Value& opcode = d["op"];
            auto discord_op = static_cast<voice_opcode>(opcode.GetInt());
            handleOpcode(discord_op, d);
            break;
        }
        default:
            DEBUG_LOG("Unhandled message type " << opcode);
            break;
    }
}

std::string voice_client::getVersionedUri(std::string endpoint)
{
    // Remove port if necessary
    auto port = endpoint.rfind(":80");
    if (port != std::string::npos)
        endpoint = endpoint.substr(0, port);

    return "wss://" + endpoint + "/?v=3";
}

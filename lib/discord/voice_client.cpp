#include <cpprest/uri_builder.h>
#include <pplx/pplx.h>

#include <thermite/discord/voice_client.hpp>
#include <thermite/logging.hpp>

#include <optional>

namespace ws = web::websockets::client;
namespace lib = thermite::discord;
using json = web::json::value;

static const web::uri get_url(std::string endpoint)
{
    auto pos = endpoint.find_last_of(':');
    if (pos != std::string::npos)
        endpoint = endpoint.substr(0, pos);

    return web::uri_builder{}
        .set_scheme("wss")
        .set_host(endpoint)
        .append_query("v", 4)
        .to_uri();
}

lib::voice_client::voice_client(std::string guild_id, std::string user_id,
    std::string endpoint, std::string session, std::string token)
    : voice_client(guild_id, user_id, endpoint, session, token,
        ws::websocket_client_config{})
{}

lib::voice_client::voice_client(std::string guild_id, std::string user_id,
    std::string endpoint, std::string session, std::string token,
        ws::websocket_client_config ws_config)
    : _guild_id{guild_id}, _user_id{user_id}, _session{session}, _token{token},
        _endpoint{get_url(endpoint)}, _ws_client{ws_config}, _udp_client{},
        _disconnect_token_source{}, _nonce{0}, _received_nonce{0}
{ }

pplx::task<void> lib::voice_client::start()
{
    if (!_hooked_events)
    {
        _hooked_events = true;
        _ws_client.set_message_handler(
            [this](const ws::websocket_incoming_message& message)
            {
                thermite::log("WS PAYLOAD RECEIVED");
                return process_websocket_event(message).wait();
            });

        _ws_client.set_close_handler([](ws::websocket_close_status status,
            const std::string reason, const std::error_code& code)
        {
            thermite::log("WS Close error: ", (short)status, " message: ",
                        reason, " error code: ",
                        code ? code.message() : "no system error occured");
        });
    }

    thermite::log("WS CONNECTING TO ", _endpoint.to_string());

    return _ws_client.connect(_endpoint)
        .then([this]
        {
            thermite::log("WS CONNECTED TO ", _endpoint.to_string());
        });
}

pplx::task<void> lib::voice_client::stop()
{
    _disconnect_token_source.cancel();
    return _ws_client.close();
}

pplx::task<void> lib::voice_client::process_websocket_event(
    const web::websockets::client::websocket_incoming_message& message)
{
    return message.extract_string()
        .then([](std::string data)
        {
            thermite::log("WS RECV: ", data);
            try
            {
                return json::parse(data);
            }
            catch (web::json::json_exception e)
            {
                thermite::log("json deserialize failed: ", e.what());
                throw;
            }
        })
        .then([this](json payload)
        {
            return process_event(payload);
        });
}

pplx::task<void> lib::voice_client::send_json(json&& value)
{
    std::string body{value.serialize()};

    thermite::log("WS SEND: ", body);

    ws::websocket_outgoing_message message;
    message.set_utf8_message(body);

    return _ws_client.send(message);
}

pplx::task<void> lib::voice_client::send_opcode(
    lib::voice_opcode opcode, json&& value)
{
    return send_json(json::object({
        {"op", json::number(static_cast<int>(opcode))},
        {"d", std::move(value)}
    }));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;

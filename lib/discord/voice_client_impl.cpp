#include <cpprest/uri_builder.h>
#include <pplx/pplx.h>
#include <sodium.h>

#include "voice_client_impl.hpp"

#include <thermite/extra/pplx_extras.hpp>
#include <thermite/logging.hpp>

#include <extra/net_utils.hpp>

namespace ws = web::websockets::client;
namespace lib = thermite::discord::detail;
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

lib::voice_client_impl::voice_client_impl(std::string guild_id,
    std::string user_id, std::string endpoint, std::string session,
    std::string token) noexcept
    : voice_client_impl(guild_id, user_id, endpoint, session, token,
        ws::websocket_client_config{})
{}

lib::voice_client_impl::voice_client_impl(std::string guild_id,
    std::string user_id, std::string endpoint, std::string session,
    std::string token, ws::websocket_client_config ws_config) noexcept
    : _guild_id{guild_id}, _user_id{user_id}, _session{session}, _token{token},
        _endpoint{get_url(endpoint)}, _ws_client{ws_config}, _udp_client{},
        _disconnect_token_source{}, _nonce{0}, _received_nonce{0}, _ssrc{0},
        _sequence{0}, _timestamp{0}, _secret_key{}
{ }

lib::voice_client_impl::~voice_client_impl()
{
    _disconnect_token_source.cancel();
    // TODO: may cause deadlocks, this should be tested
    _ws_client.close().get();
}

pplx::task<void> lib::voice_client_impl::start()
{
    if (_disconnect_token_source.get_token().is_canceled())
        _disconnect_token_source = pplx::cancellation_token_source{};

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

pplx::task<void> lib::voice_client_impl::stop()
{
    _disconnect_token_source.cancel();
    return _ws_client.close();
}

// TODO: rewrite this using pooled memory
pplx::task<std::chrono::milliseconds> lib::voice_client_impl::transmit_frame(
    thermite::memory::memory<uint8_t>&& frame, uint32_t samples)
{
    auto start = std::chrono::steady_clock::now();
    std::vector<uint8_t> payload;

    utility::push_back(payload, (uint8_t)0x80);
    utility::push_back(payload, (uint8_t)0x78);

    utility::push_back(payload, utility::to_big_endian(_sequence));
    utility::push_back(payload, utility::to_big_endian(_timestamp));
    utility::push_back(payload, utility::to_big_endian(_ssrc));

    _sequence++;
    _timestamp += samples;

    uint8_t nonce[crypto_secretbox_NONCEBYTES];
    std::memset(nonce, 0, crypto_secretbox_NONCEBYTES);
    randombytes_buf(nonce, 4);

    payload.resize(payload.size() + frame.size() + crypto_secretbox_MACBYTES);
    crypto_secretbox_easy(
        // offset for header
        payload.data() + 12,
        frame.data().span().data(),
        frame.size(),
        nonce,
        _secret_key.data()
    );
    std::move(nonce, nonce + 4, std::back_inserter(payload));

    return _udp_client.send(payload)
        .then([start](size_t)
        {
            auto finish = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                finish - start);
        });
}

pplx::task<void> lib::voice_client_impl::set_speaking(bool speaking)
{
    return send_opcode(voice_opcode::Speaking, json::object({
        {"speaking", json::number(speaking ? 1 : 0)},
        {"delay", json::number(0)},
        {"ssrc", json::number(_ssrc)}
    }));
}

pplx::task<void> lib::voice_client_impl::process_websocket_event(
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

pplx::task<void> lib::voice_client_impl::send_json(json&& value)
{
    std::string body{value.serialize()};

    thermite::log("WS SEND: ", body);

    ws::websocket_outgoing_message message;
    message.set_utf8_message(body);

    return _ws_client.send(message);
}

pplx::task<void> lib::voice_client_impl::send_opcode(
    voice_opcode opcode, json&& value)
{
    return send_json(json::object({
        {"op", json::number(static_cast<int>(opcode))},
        {"d", std::move(value)}
    }));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;

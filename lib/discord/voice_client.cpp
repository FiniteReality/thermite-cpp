#include <cpprest/uri_builder.h>
#include <pplx/pplx.h>
#include <sodium.h>

#include <thermite/discord/voice_client.hpp>
#include <thermite/logging.hpp>

#include <extra/net_utils.hpp>
#include <extra/pplx_extras.hpp>

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
        _disconnect_token_source{}, _nonce{0}, _received_nonce{0}, _ssrc{0},
        _sequence{0}, _timestamp{0}, _secret_key{}, _frame_queue{}
{
    // roughly a 5 second buffer with 20ms frames
    _frame_queue.set_capacity(250);
}

pplx::task<void> lib::voice_client::start()
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

    auto transmit_task = pplx::create_task([this]
    {
        auto runover = std::chrono::milliseconds::zero();
        while (true)
        {
            runover = transmit_frame(runover).get();
        }
    }, _disconnect_token_source.get_token());

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

pplx::task<void> lib::voice_client::queue_opus_frame(
    std::vector<uint8_t> frame, std::chrono::milliseconds length)
{
    return pplx::create_task([this, frame, length]
    {
        _frame_queue.emplace(std::move(frame), length);
    });
}

pplx::task<std::chrono::milliseconds> lib::voice_client::transmit_frame(
    std::chrono::milliseconds runover)
{
    std::pair<std::vector<uint8_t>, std::chrono::milliseconds> frame_info;
    _frame_queue.pop(frame_info);

    auto [frame, length] = frame_info;

    std::vector<uint8_t> payload;

    utility::push_back(payload, (uint8_t)0x80);
    utility::push_back(payload, (uint8_t)0x78);

    utility::push_back(payload, utility::to_big_endian(_sequence));
    utility::push_back(payload, utility::to_big_endian(_timestamp));
    utility::push_back(payload, utility::to_big_endian(_ssrc));

    _sequence++;
    _timestamp += length.count();

    uint8_t nonce[4];
    randombytes_buf(nonce, 4);

    frame.resize(frame.size() + crypto_secretbox_MACBYTES);
    crypto_secretbox_easy(
        frame.data(),
        frame.data(),
        frame.size() - crypto_secretbox_MACBYTES,
        nonce,
        _secret_key.data()
    );

    std::move(frame.begin(), frame.end(), std::back_inserter(payload));
    std::move(nonce, nonce + 4, std::back_inserter(payload));

    return _udp_client.send(payload)
        .then([length, runover](size_t)
        {
            return pplx::wait_for(length - runover);
        });
}

pplx::task<void> lib::voice_client::set_speaking(bool speaking)
{
    return send_opcode(lib::voice_opcode::Speaking, json::object({
        {"speaking", json::boolean(speaking)}
    }));
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

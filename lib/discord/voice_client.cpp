#include "voice_client_impl.hpp"

namespace ws = web::websockets::client;
namespace lib = thermite::discord;

lib::voice_client::voice_client(std::string guild_id, std::string user_id,
    std::string endpoint, std::string session, std::string token) noexcept
    : voice_client(guild_id, user_id, endpoint, session, token,
        ws::websocket_client_config{})
{}

lib::voice_client::voice_client(std::string guild_id, std::string user_id,
    std::string endpoint, std::string session, std::string token,
    ws::websocket_client_config ws_config) noexcept
    : _impl{std::make_shared<detail::voice_client_impl>(guild_id, user_id,
        endpoint, session, token, ws_config)}
{ }

pplx::task<void> lib::voice_client::start()
{
    return _impl->start();
}

pplx::task<void> lib::voice_client::stop()
{
    return _impl->stop();
}

pplx::task<void> lib::voice_client::set_speaking(bool speaking)
{
    return _impl->set_speaking(speaking);
}

pplx::task<std::chrono::milliseconds> lib::voice_client::transmit_frame(
    thermite::memory::memory<uint8_t>&& frame, uint32_t samples)
{
    return _impl->transmit_frame(std::move(frame), samples);
}

pplx::task<void> lib::voice_client::process_websocket_event(
    const web::websockets::client::websocket_incoming_message& message)
{
    return _impl->process_websocket_event(message);
}

pplx::task<void> lib::voice_client::process_event(const web::json::value& json)
{
    return _impl->process_event(json);
}
pplx::task<void> lib::voice_client::process_ready(const web::json::value& data)
{
    return _impl->process_ready(data);
}
pplx::task<void> lib::voice_client::process_session_description(
    const web::json::value& data)
{
    return _impl->process_session_description(data);
}
pplx::task<void> lib::voice_client::process_heartbeat_ack(
    const web::json::value& data)
{
    return _impl->process_heartbeat_ack(data);
}
pplx::task<void> lib::voice_client::process_hello(const web::json::value& data)
{
    return _impl->process_hello(data);
}
pplx::task<void> lib::voice_client::process_resumed(
    const web::json::value& data)
{
    return _impl->process_resumed(data);
}

pplx::task<void> lib::voice_client::do_heartbeat(
    std::chrono::milliseconds interval, std::chrono::milliseconds runover)
{
    return _impl->do_heartbeat(interval, runover);
}

pplx::task<void> lib::voice_client::send_json(web::json::value&& json)
{
    return _impl->send_json(std::move(json));
}

pplx::task<void> lib::voice_client::send_opcode(voice_opcode opcode,
    web::json::value&& json)
{
    return _impl->send_opcode(opcode, std::move(json));
}

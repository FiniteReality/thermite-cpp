#ifndef _VOICE_CLIENT_HPP_
#define _VOICE_CLIENT_HPP_

#include <cpprest/ws_client.h>
#include <cpprest/json.h>
#include <pplx/pplxtasks.h>
#include <string>

#include <thermite/memory/memory.hpp>
#include "voice_opcode.hpp"

namespace thermite::discord
{

namespace detail
{
    class voice_client_impl;
}


class voice_client
{
public:
    voice_client(std::string guild_id, std::string user_id,
        std::string endpoint, std::string session, std::string token) noexcept;

    voice_client(std::string guild_id, std::string user_id,
        std::string endpoint, std::string session, std::string token,
        web::websockets::client::websocket_client_config ws_config) noexcept;

    voice_client(const voice_client&) = default;
    voice_client& operator=(const voice_client&) = default;

    voice_client(voice_client&&) = default;
    voice_client& operator=(voice_client &&) = default;

    ~voice_client() = default;

    pplx::task<void> start();
    pplx::task<void> stop();

    pplx::task<void> set_speaking(bool speaking);

    pplx::task<std::chrono::milliseconds> transmit_frame(
        thermite::memory::memory<uint8_t>&& frame, uint32_t samples);

private:
    pplx::task<void> process_websocket_event(
        const web::websockets::client::websocket_incoming_message& message);

    pplx::task<void> process_event(const web::json::value& json);
    pplx::task<void> process_ready(const web::json::value& data);
    pplx::task<void> process_session_description(
        const web::json::value& data);
    pplx::task<void> process_heartbeat_ack(const web::json::value& data);
    pplx::task<void> process_hello(const web::json::value& data);
    pplx::task<void> process_resumed(const web::json::value& data);

    pplx::task<void> do_heartbeat(
        std::chrono::milliseconds interval,
        std::chrono::milliseconds runover);

    pplx::task<void> send_json(web::json::value&& json);

    pplx::task<void> send_opcode(voice_opcode opcode,
                                    web::json::value&& json);

    std::shared_ptr<detail::voice_client_impl> _impl;
};

}

#endif /* _VOICE_CLIENT_HPP_ */

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;

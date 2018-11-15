#ifndef _VOICE_CLIENT_IMPL_HPP_
#define _VOICE_CLIENT_IMPL_HPP_

#include <cpprest/ws_client.h>
#include <cpprest/json.h>
#include <pplx/pplxtasks.h>
#include <string>
#include <tbb/concurrent_queue.h>

#include <thermite/discord/voice_client.hpp>
#include <thermite/udp/udp_client.hpp>

namespace thermite::discord::detail
{

class voice_client_impl
{
public:
    voice_client_impl(std::string guild_id, std::string user_id,
        std::string endpoint, std::string session, std::string token) noexcept;

    voice_client_impl(std::string guild_id, std::string user_id,
        std::string endpoint, std::string session, std::string token,
        web::websockets::client::websocket_client_config ws_config) noexcept;

    voice_client_impl(const voice_client_impl&) = default;
    voice_client_impl& operator=(const voice_client_impl&) = default;

    voice_client_impl(voice_client_impl&&) = default;
    voice_client_impl& operator=(voice_client_impl &&) = default;

    ~voice_client_impl();

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
    const std::string _guild_id, _user_id, _session, _token;
    const web::uri _endpoint;
    bool _hooked_events;

    web::websockets::client::websocket_callback_client _ws_client;
    thermite::udp::udp_client _udp_client;
    pplx::cancellation_token_source _disconnect_token_source;

    // TODO: these may benefit from either locking or atomicity
    uint32_t _nonce;
    uint32_t _received_nonce;
    uint32_t _ssrc;
    uint16_t _sequence;
    uint32_t _timestamp;
    std::vector<uint8_t> _secret_key;

    friend voice_client;
};

}

#endif /* _VOICE_CLIENT_IMPL_HPP_ */

#ifndef _VOICE_CLIENT_HPP_
#define _VOICE_CLIENT_HPP_

#include <cpprest/ws_client.h>
#include <cpprest/json.h>
#include <pplx/pplxtasks.h>
#include <string>
#include <tbb/concurrent_queue.h>

#include <thermite/udp/udp_client.hpp>
#include "voice_opcode.hpp"

namespace thermite::discord
{

class voice_client
{
    public:
        voice_client(std::string guild_id, std::string user_id,
            std::string endpoint, std::string session, std::string token);

        voice_client(std::string guild_id, std::string user_id,
            std::string endpoint, std::string session, std::string token,
            web::websockets::client::websocket_client_config ws_config);

        voice_client(const voice_client&) = default;
        voice_client& operator=(const voice_client&) = default;

        voice_client(voice_client&&) = default;
        voice_client& operator=(voice_client &&) = default;

        ~voice_client() = default;

        pplx::task<void> start();
        pplx::task<void> stop();

        pplx::task<void> queue_opus_frame(std::vector<uint8_t> data,
            std::chrono::milliseconds length);

    private:
        pplx::task<std::chrono::milliseconds> transmit_frame(
            std::chrono::milliseconds length);
        pplx::task<void> set_speaking(bool speaking);

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
        uint32_t _sequence;
        uint32_t _timestamp;
        std::vector<uint8_t> _secret_key;
        tbb::concurrent_bounded_queue<std::pair<std::vector<uint8_t>,
            std::chrono::milliseconds>> _frame_queue;
};

}

#endif /* _VOICE_CLIENT_HPP_ */

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;

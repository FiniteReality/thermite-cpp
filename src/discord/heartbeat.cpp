#include <thermite.hpp>
#include <thermite/voice_client.hpp>

#include "internal.hpp"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace thermite::discord;

void voice_client::beginHeartbeatThread(
    int intervalMs)
{
    // work around a discord bug
    intervalMs = intervalMs * 0.75;

    uv_timer_init(detail::get_uv_loop(), &_heartbeat_timer);
    _heartbeat_timer.data = this;

    uv_timer_start(&_heartbeat_timer, [](uv_timer_t* timer)
    {
        DEBUG_LOG("sending heartbeat");
        auto self = static_cast<voice_client*>(timer->data);

        if (self->_last_received_nonce != self->_nonce)
        {
            DEBUG_LOG(
                "server missed last heartbeat, nonce mismatch: " <<
                self->_nonce << " (current) vs " <<
                self->_last_received_nonce << " (last received).");
        }

        rapidjson::Document d;
        auto& allocator = d.GetAllocator();
        d.SetObject();

        d.AddMember("op", static_cast<int>(voice_opcode::Heartbeat), allocator);
        d.AddMember("d", ++self->_nonce, allocator);

        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        d.Accept(writer);

        self->_websocket->send(buf.GetString(), buf.GetSize(), uWS::OpCode::TEXT);
    }, 0, intervalMs);
}

void voice_client::handleHeartbeatAck(uint64_t nonce)
{
    if (_nonce != nonce)
    {
        DEBUG_LOG(
            "server missed last heartbeat, nonce mismatch: " <<
            _nonce << " (current) vs " << nonce << " (received).");
    }

    _last_received_nonce = nonce;
}

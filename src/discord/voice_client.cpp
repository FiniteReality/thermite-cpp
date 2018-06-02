#include <thermite.hpp>
#include <thermite/voice_client.hpp>

#include "internal.hpp"

#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

using namespace thermite::discord;

voice_client::voice_client()
    : _heartbeatTimer {}, _socket{}, _webSocket{}, _resuming{false},
    _serverId{}, _userId{}, _endpoint{}, _sessionId{}, _token{}, _nonce{0},
    _lastReceivedNonce {0}, _sendAddr {}, _mode{voice_mode::Unknown},
    _secret{}, _sequence{0}, _timestamp{0}, _ssrc{0}, _lastKeepalive{0}
{
    uv_timer_init(detail::getUvLoop(), &_heartbeatTimer);
    _heartbeatTimer.data = this;

    uv_udp_init(detail::getUvLoop(), &_socket);
    _socket.data = this;

    beginUdpReceive();
}

voice_client::~voice_client()
{
    disconnect();
    uv_close(reinterpret_cast<uv_handle_t*>(&_heartbeatTimer), nullptr);
    uv_close(reinterpret_cast<uv_handle_t*>(&_socket), nullptr);
}

void voice_client::connect(
    const std::string& server_id,
    const std::string& user_id,
    const std::string& endpoint,
    const std::string& session,
    const std::string& token)
{
    _nonce = 0;
    _lastReceivedNonce = 0;

    _serverId = server_id;
    _userId = user_id;
    _sessionId = session;
    _token = token;

    _endpoint = getVersionedUri(endpoint);
    DEBUG_LOG("connecting to: " << _endpoint);
    detail::connect(_endpoint, this);
}

void voice_client::disconnect()
{
    stopUdpReceive();
    uv_timer_stop(&_heartbeatTimer);
    _webSocket->close();
}

void voice_client::set_speaking(bool speaking)
{
    sendSpeaking(speaking);
}

void voice_client::onWSConnect(uWS::WebSocket<false>* client)
{
    this->_webSocket = client;
}

void voice_client::onWSDisconnect(int code, const std::string& message)
{
    DEBUG_LOG("WebSocket close: " << code << " (" << message << ")");

    disconnect();

    _resuming = !_resuming;

    DEBUG_LOG("connecting to: " << _endpoint);
    detail::connect(_endpoint, this);
}

void voice_client::sendWSMessage(rapidjson::Document& document)
{
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer{buf};
    document.Accept(writer);

    DEBUG_LOG("SEND: " << buf.GetString());

    _webSocket->send(buf.GetString(), buf.GetSize(), uWS::OpCode::TEXT);
}

void voice_client::onWSMessage(
    const char* data,
    size_t count,
    uWS::OpCode opcode)
{
    std::string msg{data, count};
    DEBUG_LOG("RECV: " << msg);

    switch (opcode)
    {
        case uWS::OpCode::TEXT:
        {
            rapidjson::Document d;
            d.Parse(data, count);
            rapidjson::Value& opcode = d["op"];
            auto discord_op = static_cast<voice_opcode>(opcode.GetInt());
            onOpcode(discord_op, d);
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

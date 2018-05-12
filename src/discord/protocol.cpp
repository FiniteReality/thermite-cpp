#include <algorithm>

#include <thermite.hpp>
#include <thermite/voice_client.hpp>

#include "internal.hpp"

using namespace thermite::discord;

void voice_client::onOpcode(
    voice_opcode opcode,
    rapidjson::Document& document)
{
    switch (opcode)
    {
        case voice_opcode::Hello:
        {
            int interval = document["d"]["heartbeat_interval"].GetInt();

            // work around a discord bug
            interval *= 0.75;

            uv_timer_start(&_heartbeatTimer, [](uv_timer_t* timer)
            {
                auto self = static_cast<voice_client*>(timer->data);

                if (self->_lastReceivedNonce != self->_nonce)
                {
                    DEBUG_LOG(
                        "Server missed last heartbeat, nonce mismatch: " <<
                        self->_nonce << " (current) vs " <<
                        self->_lastReceivedNonce << " (last received).");
                }

                self->sendHeartbeat();
            }, 0, interval);

            sendIdentify();
            break;
        }
        case voice_opcode::Ready:
        {
            auto& d = document["d"];
            _ssrc = d["ssrc"].GetInt();

            auto ip = d["ip"].GetString();
            auto port = d["port"].GetInt();

            // TODO: figure out what this returns
            uv_ip4_addr(ip, port, &_sendAddr);

            negotiateEncryptionMode(d["modes"].GetArray());
            break;
        }
        case voice_opcode::SessionDescription:
        {
            auto& d = document["d"];
            auto secret = d["secret_key"].GetArray();
            _secret.resize(secret.Size());
            std::transform(secret.begin(), secret.end(), _secret.begin(),
                [](rapidjson::Value& elem)
                {
                    return elem.GetInt();
                });
            break;
        }
        case voice_opcode::HeartbeatAck:
        {
            uint64_t nonce = document["d"].GetUint64();

            if (_nonce != nonce)
            {
                DEBUG_LOG(
                    "Server missed last heartbeat, nonce mismatch: " <<
                    _nonce << " (current) vs " << nonce << " (received).");
            }

            _lastReceivedNonce = nonce;

            break;
        }
        default:
            DEBUG_LOG("Unhandled opcode " << opcode);
            break;
    }
}

void voice_client::sendHeartbeat()
{
    DEBUG_LOG("Sending heartbeat");

    sendOpcode<voice_opcode::Heartbeat>(rapidjson::Value{++_nonce});
}

void voice_client::sendIdentify()
{
    DEBUG_LOG("Sending identify");

    rapidjson::Document identify(rapidjson::kObjectType);
    auto& allocator = identify.GetAllocator();

    rapidjson::Value serverId;
    serverId.SetString(rapidjson::StringRef(_serverId));
    rapidjson::Value userId;
    userId.SetString(rapidjson::StringRef(_userId));
    rapidjson::Value sessionId;
    sessionId.SetString(rapidjson::StringRef(_sessionId));
    rapidjson::Value token;
    token.SetString(rapidjson::StringRef(_token));

    identify.AddMember("server_id", std::move(serverId), allocator);
    identify.AddMember("user_id", std::move(userId), allocator);
    identify.AddMember("session_id", std::move(sessionId), allocator);
    identify.AddMember("token", std::move(token), allocator);

    sendOpcode<voice_opcode::Identify>(std::move(identify));
}

void voice_client::sendSpeaking(bool isSpeaking)
{
    rapidjson::Document speaking(rapidjson::kObjectType);
    auto& allocator = speaking.GetAllocator();

    speaking.AddMember("speaking", isSpeaking, allocator);
    speaking.AddMember("delay", 0, allocator);
    speaking.AddMember("ssrc", _ssrc, allocator);

    sendOpcode<voice_opcode::Speaking>(std::move(speaking));
}

void voice_client::negotiateEncryptionMode(
    const voice_client::rapidJsonArray& modes)
{
#define contains(arr, value) \
    (std::find((arr).begin(), (arr).end(), (value)) != (arr).end())

    _mode = voice_mode::Unknown;

    // Use libsodium's xsalsa20 stream cipher
    if (contains(modes, "xsalsa20_poly1305"))
    {
        _mode = voice_mode::XSalsa20_Poly1305;
    }

    // Use Discord's 'suffix' version of the xsalsa20 stream cipher
    if (contains(modes, "xsalsa20_poly1305_suffix"))
    {
        _mode = voice_mode::XSalsa20_Poly1305_Suffix;
    }

    // Use Discord's 'lite' version of the xsalsa20 stream cipher
    if (contains(modes, "xsalsa20_poly1305_lite"))
    {
        _mode = voice_mode::XSalsa20_Poly1305_Lite;
    }

    // Nothing else is supported, throw an error
    if (_mode == voice_mode::Unknown)
    {
        throw std::runtime_error{"none of the modes provided were supported"};
    }

    sendDiscovery();
#undef contains
}

void voice_client::finishNegotiateEncryptionMode(
    std::string ip, unsigned short port)
{
    DEBUG_LOG("Sending protocol selection");

    rapidjson::Document d{rapidjson::kObjectType};
    rapidjson::Value data{rapidjson::kObjectType};
    auto& allocator = d.GetAllocator();

    d.AddMember("protocol", "udp", allocator);
    data.AddMember("port", port, allocator);
    data.AddMember("ip",
        rapidjson::Value{}.SetString(ip.c_str(), ip.length()),
        allocator);

    std::string mode;
    switch (_mode) {
        case voice_mode::XSalsa20_Poly1305:
            mode = "xsalsa20_poly1305";
            break;
        case voice_mode::XSalsa20_Poly1305_Suffix:
            mode = "xsalsa20_poly1305_suffix";
            break;
        case voice_mode::XSalsa20_Poly1305_Lite:
            mode = "xsalsa20_poly1305_lite";
            break;
        default: // Will never happen, shuts up semantic analysis
            break;
    }

    data.AddMember("mode",
        rapidjson::Value{}.SetString(mode.c_str(), mode.length()),
        allocator);
    d.AddMember("data", data, allocator);

    sendOpcode<voice_opcode::SelectProtocol>(std::move(d));
}

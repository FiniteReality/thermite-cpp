#include <algorithm>

#include <thermite.hpp>
#include <thermite/voice_client.hpp>

#include "internal.hpp"

using namespace thermite::discord;

void voice_client::handleOpcode(
    voice_opcode opcode,
    rapidjson::Document& document)
{
    switch (opcode)
    {
        case voice_opcode::Hello:
        {
            int interval = document["d"]["heartbeat_interval"].GetInt();
            beginHeartbeatThread(interval);
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
            uv_ip4_addr(ip, port, &_send_addr);

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
            handleHeartbeatAck(nonce);
            break;
        }
        default:
            DEBUG_LOG("Unhandled opcode " << opcode);
            break;
    }
}

void voice_client::sendIdentify()
{
    DEBUG_LOG("Sending identify");

    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Value d{rapidjson::kObjectType};
    auto& allocator = doc.GetAllocator();

    d.AddMember("server_id",
        rapidjson::Value{}.SetString(_server_id.c_str(), _server_id.length()),
        allocator);
    d.AddMember("user_id",
        rapidjson::Value{}.SetString(_user_id.c_str(), _user_id.length()),
        allocator);
    d.AddMember("session_id",
        rapidjson::Value{}.SetString(_session.c_str(), _session.length()),
        allocator);
    d.AddMember("token",
        rapidjson::Value{}.SetString(_token.c_str(), _token.length()),
        allocator);

    doc.AddMember("op", static_cast<int>(voice_opcode::Identify), allocator);
    doc.AddMember("d", d, allocator);

    sendWSMessage(doc);
}

void voice_client::negotiateEncryptionMode(
    const voice_client::rapidJsonArray& modes)
{
#define contains(arr, value) \
    (std::find((arr).begin(), (arr).end(), (value)) != (arr).end())

    _mode = voice_mode::Unknown;

    // Use Discord's 'lite' version of the xsalsa20 stream cipher
    if (contains(modes, "xsalsa20_poly1305_lite"))
    {
        _mode = voice_mode::XSalsa20_Poly1305_Lite;
    }

    // Use Discord's 'suffix' version of the xsalsa20 stream cipher
    if (contains(modes, "xsalsa20_poly1305_suffix"))
    {
        _mode = voice_mode::XSalsa20_Poly1305_Suffix;
    }

    // Use libsodium's xsalsa20 stream cipher
    if (contains(modes, "xsalsa20_poly1305"))
    {
        _mode = voice_mode::XSalsa20_Poly1305;
    }

    // Nothing else is supported, throw an error
    if (_mode == voice_mode::Unknown)
    {
        throw std::runtime_error{"none of the modes provided were supported"};
    }

    uv_udp_init(detail::get_uv_loop(), &_socket);
    _socket.data = this;

    sendDiscovery();
#undef contains
}

void voice_client::finishNegotiateEncryptionMode(
    std::string ip, unsigned short port)
{
    DEBUG_LOG("Sending protocol selection");

    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Value d{rapidjson::kObjectType};
    rapidjson::Value data{rapidjson::kObjectType};
    auto& allocator = doc.GetAllocator();

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
    doc.AddMember("op", static_cast<int>(voice_opcode::SelectProtocol), allocator);
    doc.AddMember("d", d, allocator);

    sendWSMessage(doc);
}

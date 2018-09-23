#include <algorithm>
#include <stdexcept>
#include <thermite/discord/voice_client.hpp>
#include <thermite/extra/pplx_extras.hpp>
#include <thermite/logging.hpp>

#include <extra/net_utils.hpp>

namespace ws = web::websockets::client;
namespace lib = thermite::discord;
using json = web::json::value;

pplx::task<void> lib::voice_client::process_event(const json& json)
{
    return pplx::create_task(
        [this, json]()
        {
            int op_int = json.at("op").as_integer();
            auto op = static_cast<voice_opcode>(op_int);

            switch(op)
            {
                case voice_opcode::Identify:
                    throw std::domain_error(
                        "received op 0 (VOICE_IDENTIFY) unexpectedly");
                case voice_opcode::SelectProtocol:
                    throw std::domain_error(
                        "received op 1 (SELECT_PROTOCOL) unexpectedly");
                case voice_opcode::Ready:
                    return process_ready(json.at("d"));
                case voice_opcode::Heartbeat:
                    throw std::domain_error(
                        "received op 3 (HEARTBEAT) unexpectedly");
                case voice_opcode::SessionDescription:
                    return process_session_description(json.at("d"));
                case voice_opcode::Speaking:
                    return pplx::completed_task();
                case voice_opcode::HeartbeatAck:
                    return process_heartbeat_ack(json.at("d"));
                case voice_opcode::ResumeConnection:
                    throw std::domain_error(
                        "received op 7 (RESUME_CONNECTION) unexpectedly");
                case voice_opcode::Hello:
                    return process_hello(json.at("d"));
                case voice_opcode::Resumed:
                    return process_resumed(json.at("d"));
                default:
                    thermite::log("unhandled opcode ", op_int);
                    return pplx::completed_task();
            }
        })
        .then([](pplx::task<void> task)
        {
            try
            {
                task.get();
            }
            catch (web::json::json_exception e)
            {
                thermite::log("json exception occured: ",  e.what());
                throw;
            }
        });
}

pplx::task<void> lib::voice_client::process_ready(const json& data)
{
    _ssrc = data.at("ssrc").as_number().to_uint32();

    auto ip = data.at("ip").as_string();
    auto port = data.at("port").as_number().to_uint32();

    auto modes = data.at("modes").as_array();

    bool compatible = false;
    for (auto& item : modes)
    {
        if (item.is_string() && item.as_string() == "xsalsa20_poly1305_lite")
        {
            compatible = true;
            break;
        }
    }

    if (compatible)
    {
        return _udp_client.connect(ip, port)
            .then([this]
            {
                std::vector<uint8_t> discovery;
                utility::push_back(discovery, utility::to_big_endian(_ssrc));
                discovery.resize(70);

                return _udp_client.send(discovery)
                    .then([discovery](std::size_t){});
            })
            .then([ip, port, this]
            {
                return _udp_client.receive_from(ip, port, 70);
            })
            .then([](std::vector<uint8_t> data)
            {
                if (data.size() >= 70)
                {
                    std::string our_ip{reinterpret_cast<char*>(&data[4])};
                    uint16_t our_port = *(reinterpret_cast<uint16_t*>(
                        &data[68]));

                    our_port = utility::to_little_endian(our_port);

                    return std::make_pair(our_ip, our_port);
                }
                else
                {
                    // TODO: do something better than this
                    throw std::runtime_error("didn't receive 70 bytes");
                }
            })
            .then([this](std::pair<std::string, uint16_t> our_ip_info)
            {
                return send_opcode(voice_opcode::SelectProtocol, json::object({
                    {"protocol", json::string("udp")},
                    {"data", json::object({
                        {"ip", json::string(our_ip_info.first)},
                        {"port", json::number(our_ip_info.second)},
                        {"mode", json::string("xsalsa20_poly1305_lite")}
                    })},
                }));
            });
    }
    else
    {
        throw std::runtime_error("xsalsa20_poly1305_lite was not supported");
    }
}

pplx::task<void> lib::voice_client::process_session_description(
    const json& data)
{
    auto secret_key = data.at("secret_key").as_array();
    std::transform(secret_key.begin(), secret_key.end(),
        std::back_inserter(_secret_key),
        [](const json& val)
        {
            return val.as_number().to_int32();
        });

    thermite::log("Processed session description, got secret key at ",
        _secret_key.size(), " bytes");

    return pplx::completed_task();
}

pplx::task<void> lib::voice_client::process_heartbeat_ack(const json& data)
{
    _received_nonce = data.as_integer();
    return pplx::completed_task();
}

pplx::task<void> lib::voice_client::process_hello(const json& data)
{
    auto interval_ms = (int)data.at("heartbeat_interval")
        .as_number()
        .to_double();

    auto heartbeat_task = pplx::create_task([this, interval_ms]
    {
        return do_heartbeat(std::chrono::milliseconds(interval_ms),
            std::chrono::milliseconds::zero());
    }, _disconnect_token_source.get_token());

    return send_opcode(voice_opcode::Identify, json::object({
        {"server_id", json::string(_guild_id)},
        {"user_id", json::string(_user_id)},
        {"session_id", json::string(_session)},
        {"token", json::string(_token)}
    }));
}

pplx::task<void> lib::voice_client::process_resumed(const json&)// data)
{
    return pplx::completed_task();
}

pplx::task<void> lib::voice_client::do_heartbeat(
    std::chrono::milliseconds interval,
    std::chrono::milliseconds runover)
{
    auto delta = _nonce - _received_nonce;
    if (delta > 1)
    {
        thermite::log("Nonce mismatch on heartbeat: last received ",
                      _received_nonce, " expecting ", _nonce);
        throw std::runtime_error("nonce mismatch");
    }

    return pplx::wait_for(std::chrono::milliseconds(interval) - runover)
        .then([this, interval](std::chrono::milliseconds runover)
        {
            return send_opcode(voice_opcode::Heartbeat,
                json::number(++_nonce))
                .then([this, interval, runover]
                {
                    return do_heartbeat(interval, runover);
                });
        });
}

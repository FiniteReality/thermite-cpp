#ifndef VOICE_CLIENT_HPP
#define VOICE_CLIENT_HPP

#include "../thermite.hpp"
#include "discord/voice_opcode.hpp"
#include "discord/voice_mode.hpp"

#include <rapidjson/document.h>
#include <uv.h>
#include <uWS/uWS.h>

#include <map>
#include <string>

namespace thermite
{
namespace discord
{
namespace detail
{

void onWSMessage(
    uWS::WebSocket<false>* wsClient,
    const char* message,
    size_t size,
    uWS::OpCode opcode);

void onWSConnect(
    uWS::WebSocket<false>* wsClient,
    uWS::HttpRequest);

void onSocketData(
    uv_udp_t* handle,
    ssize_t read,
    const uv_buf_t* buf,
    const struct sockaddr* addr,
    unsigned flags);

}

class voice_client
{
public:
    /*!\brief Connects to a Discord voice server with the given endpoint and
     *        authentication.
     *
     * \param server_id The id of the server to connect to
     * \param user_id The id of the user to authenticate as
     * \param endpoint The endpoint of the voice server as given by Discord
     * \param session The session id to handshake with
     * \param token The token to authenticate with
     */
    void connect(
        std::string server_id,
        std::string client_id,
        std::string endpoint,
        std::string session,
        std::string token);

    /*!\brief Disconnects from Discord
     */
    void disconnect();

    void play_file(std::string location);

private:
    using rapidJsonArray = rapidjson::Document::ValueType::Array;

    bool _listen;
    uv_timer_t _heartbeat_timer;
    uv_udp_t _socket;
    uWS::WebSocket<false>* _websocket;

    std::string _server_id;
    std::string _user_id;
    std::string _session;
    std::string _token;

    uint64_t _nonce;
    uint64_t _last_received_nonce;
    int _ssrc;
    struct sockaddr_in _send_addr;
    voice_mode _mode;
    std::vector<unsigned char> _secret;

    void sendWSMessage(rapidjson::Document& document);

    void onSocketMessage(
        ssize_t read,
        const uv_buf_t* buf,
        const struct sockaddr* addr,
        unsigned flags);

    void onWSMessage(
        const char* data,
        size_t count,
        uWS::OpCode opcode);

    void handleOpcode(
            voice_opcode opcode,
        rapidjson::Document& document);

    void handleHeartbeatAck(uint64_t nonce);
    void beginHeartbeatThread(
        int intervalMs);

    void sendIdentify();

    void negotiateEncryptionMode(
        const rapidJsonArray& modes);
    void finishNegotiateEncryptionMode(std::string ip, unsigned short port);

    void sendDiscovery();

    static std::string getVersionedUri(std::string endpoint);

    friend void detail::onWSMessage(
        uWS::WebSocket<false>* wsClient,
        const char* data,
        size_t size,
        uWS::OpCode opcode);

    friend void detail::onWSConnect(
        uWS::WebSocket<false>* wsClient,
        uWS::HttpRequest);

    friend void detail::onSocketData(
        uv_udp_t* handle,
        ssize_t read,
        const uv_buf_t* buf,
        const struct sockaddr*,
        unsigned);
};

}
}

#endif /* VOICE_CLIENT_HPP */

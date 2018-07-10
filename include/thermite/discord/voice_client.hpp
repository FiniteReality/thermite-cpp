#ifndef VOICE_CLIENT_HPP
#define VOICE_CLIENT_HPP

#include "voice_opcode.hpp"
#include "voice_mode.hpp"
#include "voice_callbacks.hpp"

#include <rapidjson/document.h>
#include <string>
#include <vector>

namespace thermite
{
namespace discord
{

class voice_client
{
public:
    voice_client();

    ~voice_client();

    voice_client(const voice_client& other) = delete;
    voice_client(voice_client&& other) = delete;
    voice_client& operator=(const voice_client& other) = delete;
    voice_client& operator=(voice_client&& other) = delete;

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
        const std::string& server_id,
        const std::string& user_id,
        const std::string& endpoint,
        const std::string& session,
        const std::string& token);

    /*!\brief Disconnects from Discord.
     */
    void disconnect();

    /*!\brief Updates the speaking status with Discord.
     *
     * \param speaking Set to true to indicate that we intend to send UDP data
     * \param delay_ms The amount of delay added by Opus
     */
    void set_speaking(bool speaking, uint32_t delay_ms = 0);

    /*!\brief Sends an opus packet to Discord.
     *
     * \param frame The encoded Opus frame to queue
     * \param frame_samples The number of samples the frame contains
     */
    void send_opus_packet(
        const std::vector<unsigned char>& frame,
        uint32_t frame_samples);

private:
    using rapidJsonArray = rapidjson::Document::ValueType::Array;

    uv_timer_t _heartbeatTimer;
    uv_udp_t _socket;
    uWS::WebSocket<false>* _webSocket;
    bool _resuming;

    std::string _serverId;
    std::string _userId;
    std::string _endpoint;
    std::string _sessionId;
    std::string _token;

    uint64_t _nonce;
    uint64_t _lastReceivedNonce;
    struct sockaddr_in _sendAddr;
    voice_mode _mode;

    std::vector<unsigned char> _secret;
    uint16_t _sequence;
    uint32_t _timestamp;
    int32_t _ssrc;
    uint64_t _lastKeepalive;

    // GENERAL
    static std::string getVersionedUri(std::string endpoint);

    // OUTGOING
    void sendWSMessage(rapidjson::Document& doc);

    template <voice_opcode opcode>
    void sendOpcode(rapidjson::Value&& data)
    {
        rapidjson::Document doc;
        auto& allocator = doc.GetAllocator();
        doc.SetObject();

        doc.AddMember("op", static_cast<int>(opcode), allocator);
        doc.AddMember("d", std::move(data), allocator);

        sendWSMessage(doc);
    }

    // WEBSOCKET METHODS
    void sendHeartbeat();
    void sendResume();
    void sendIdentify();
    void sendSpeaking(bool isSpeaking, uint32_t delayMs);

    void negotiateEncryptionMode(const rapidJsonArray& modes);
    void finishNegotiateEncryptionMode(std::string ip, unsigned short port);

    // UDP METHODS
    void sendDiscovery();

    void beginUdpReceive();
    void stopUdpReceive();

    // EVENTS
    void onWSConnect(uWS::WebSocket<false>* client);
    void onWSDisconnect(int code, const std::string& message);

    void onSocketMessage(
        ssize_t read,
        const uv_buf_t* buf,
        const struct sockaddr* addr,
        unsigned flags);
    void onWSMessage(
        const char* data,
        size_t count,
        uWS::OpCode opcode);

    void onOpcode(voice_opcode opcode, rapidjson::Document& document);

    // GLOBAL EVENTS
    friend void detail::onWSMessage(
        uWS::WebSocket<false>* wsClient,
        const char* data,
        size_t size,
        uWS::OpCode opcode);
    friend void detail::onWSConnect(
        uWS::WebSocket<false>* wsClient,
        uWS::HttpRequest);
    friend void detail::onWSDisconnect(
        uWS::WebSocket<false>* wsClient,
        int code,
        const char* message,
        size_t length);
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

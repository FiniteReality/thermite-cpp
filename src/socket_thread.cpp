#include <thermite.hpp>
#include "discord/internal.hpp"

#include <uWS/uWS.h>

using namespace thermite::discord;

static uWS::Hub socketHub;
static std::map<std::string, std::string> headers;

void thermite::start_socket_thread()
{
    DEBUG_LOG("running event loop");
    socketHub.run();
    DEBUG_LOG("event loop returned?");
}

uv_loop_t* detail::get_uv_loop()
{
    return socketHub.getLoop();
}

void detail::connect(std::string uri, discord::voice_client* client)
{
    socketHub.connect(uri, client, headers);
}

class setup
{
public:
    setup()
    {
        #define GetBotHeader(url, version) \
        "DiscordBot (" url ", " version ")"

        headers["User-Agent"] = GetBotHeader(THERMITE_URL, THERMITE_VERSION);

        #undef GetBotHeader

        socketHub.onConnection(detail::onWSConnect);

        socketHub.onDisconnection([](
            uWS::WebSocket<false>* client,
            int code,
            const char* message,
            size_t length)
        {
            std::string message_s{message, length};
            DEBUG_LOG("Socket closed: " << code << " (" << message_s << ")");
        });

        socketHub.onMessage(detail::onWSMessage);

        socketHub.onError([](void*)
        {
            assert(false);
        });
    }
};

static setup _setterUpper;

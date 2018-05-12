#include <thermite.hpp>
#include "discord/internal.hpp"

#include <uWS/uWS.h>

using namespace thermite::discord;

static uWS::Hub socketHub;
static std::map<std::string, std::string> headers;

void thermite::start_event_loop()
{
    DEBUG_LOG("running event loop");
    socketHub.run();
    DEBUG_LOG("event loop returned?");
}

uv_loop_t* detail::getUvLoop()
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
        #define getBotHeader(url, version) \
            "DiscordBot (" url ", " version ")"

        headers["User-Agent"] = getBotHeader(THERMITE_URL, THERMITE_VERSION);

        #undef GetBotHeader

        socketHub.onConnection(detail::onWSConnect);
        socketHub.onDisconnection(detail::onWSDisconnect);

        socketHub.onMessage(detail::onWSMessage);

        socketHub.onError([](void*)
        {
            assert(false);
        });
    }
};

static setup _setterUpper;

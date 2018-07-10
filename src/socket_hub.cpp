#include <thermite.hpp>
#include <thermite/discord/voice_callbacks.hpp>

#include "debug.hpp"
#include "discord/internal.hpp"

#include <assert.h>
#include <uWS/uWS.h>

using namespace thermite;

static uWS::Hub socketHub;
static std::map<std::string, std::string> headers;

void thermite::start_event_loop()
{
    DEBUG_LOG("running event loop");
    socketHub.run();
    DEBUG_LOG("event loop returned?");
}

uv_loop_t* thermite::detail::getUvLoop()
{
    return socketHub.getLoop();
}

void discord::detail::connect(std::string uri, discord::voice_client* client)
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

        socketHub.onConnection(discord::detail::onWSConnect);
        socketHub.onDisconnection(discord::detail::onWSDisconnect);

        socketHub.onMessage(discord::detail::onWSMessage);

        socketHub.onError([](void*)
        {
            assert(false);
        });
    }
};

static setup _setterUpper;

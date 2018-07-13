#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "discord/voice_client.hpp"
#include "player/player_state.hpp"

#include <functional>
#include <tbb/concurrent_queue.h>
#include <thread>
#include <uv.h>
#include <vector>

namespace thermite
{

/**
 * \brief An interface for managing the player state of a specific voice client
 */
class player
{
private:
    explicit player(discord::voice_client client);
    ~player() = default;

    player(player&&) = default;
    player(const player&) = default;
    player& operator=(player&&) = default;
    player& operator=(const player&) = default;

public:
    void play();
    void pause();

    void disconnect();

    std::vector<std::function<void(std::vector<unsigned char>&)>>& filters();

private:
    std::vector<std::function<void(std::vector<unsigned char>&)>> _filters;
    discord::voice_client _client;
    tbb::concurrent_bounded_queue<std::vector<unsigned char>> _frames;
    std::thread _transcodeThread;
    uv_timer_t _broadcastTimer;
    player_state _state;
};

}

#endif /* PLAYER_HPP */

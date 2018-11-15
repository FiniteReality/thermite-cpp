#ifndef _PLAYER_HPP_
#define _PLAYER_HPP_

#include <atomic>
#include <pplx/pplxtasks.h>
#include <tbb/concurrent_queue.h>
#include <thread>

#include "audio/transcoder.hpp"
#include "discord/voice_client.hpp"

namespace thermite
{

class player
{
public:
    explicit player() = default;

    pplx::task<void> play();
    pplx::task<void> stop();

private:
    pplx::task<void> transmit(const pplx::cancellation_token& token
        = pplx::cancellation_token::none());
    pplx::task<void> transcode(const pplx::cancellation_token& token
        = pplx::cancellation_token::none());

    std::atomic_bool _playing;
    discord::voice_client& _client;
    std::unique_ptr<audio::transcoder> _transcoder;
    pplx::cancellation_token_source _cancel_token_source;
    pplx::task<void> _transmit_task;
    pplx::task<void> _transcode_task;
    tbb::concurrent_bounded_queue<
        thermite::memory::memory<uint8_t>> _frames;
};

}

#endif /* _PLAYER_HPP_ */

#include <thermite/extra/pplx_extras.hpp>
#include <thermite/player.hpp>

const int frame_ms = 20;

static std::vector<uint8_t> silence_frames{
    0xF8, 0xFF, 0xFE, 0xF8, 0xFF, 0xFE,
    0xF8, 0xFF, 0xFE, 0xF8, 0xFF, 0xFE,
    0xF8, 0xFF, 0xFE
};

pplx::task<void> thermite::player::play()
{
    auto playing = false;
    if (_playing.compare_exchange_strong(playing, true))
    {
        pplx::cancellation_token_source new_source;
        _cancel_token_source = new_source;
        return _client.set_speaking(true)
            .then([this]()
            {
                _transmit_task = transmit(_cancel_token_source.get_token());
                _transcode_task = transcode(_cancel_token_source.get_token());
            });
    }

    return pplx::completed_task();
}

pplx::task<void> thermite::player::stop()
{
    auto playing = true;
    if (_playing.compare_exchange_strong(playing, false))
    {
        _cancel_token_source.cancel();
        return _transmit_task
            .then([this]()
            {
                return _transcode_task;
            })
            .then([this]()
            {
                return _client.set_speaking(false);
            });
    }

    return pplx::completed_task();
}

pplx::task<void> thermite::player::transmit(
    const pplx::cancellation_token& token)
{
    if (token.is_canceled())
        return _client.transmit_frame(
            thermite::memory::memory(silence_frames), 5)
            .then([](std::chrono::milliseconds){});

    thermite::memory::memory<uint8_t> frame;
    pplx::task<std::chrono::milliseconds> tsk;

    if (_frames.try_pop(frame))
        tsk = _client.transmit_frame(std::move(frame), 0);
    else
        tsk = _client.transmit_frame(
            thermite::memory::memory(silence_frames), 5);

    return tsk
        .then([token](std::chrono::milliseconds delay)
        {
            return pplx::wait_for(std::chrono::milliseconds(frame_ms) - delay);
        })
        .then([this,token](std::chrono::milliseconds)
        {
            return transmit(token);
        });
}

pplx::task<void> thermite::player::transcode(
    const pplx::cancellation_token& token)
{
    if (token.is_canceled())
        return pplx::completed_task();

    _frames.push(_transcoder->transcode(std::chrono::milliseconds(frame_ms)));

    return transcode(token);
}

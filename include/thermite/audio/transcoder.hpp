#ifndef _TRANSCODER_HPP_
#define _TRANSCODER_HPP_

#include <chrono>

#include <thermite/memory/memory.hpp>

namespace thermite::audio
{

class transcoder
{
public:
    virtual thermite::memory::memory<uint8_t> transcode(
        const std::chrono::milliseconds length) = 0;
};

}

#endif /* _TRANSCODER_HPP_ */

#ifndef THERMITE_HPP
#define THERMITE_HPP

// Macros for making sure uWS uses the correct impl
#define USE_LIBUV
#undef USE_EPOLL

#define THERMITE_URL "https://github.com/FiniteReality/thermite"
#define THERMITE_VERSION "0.0.1"

#include "thermite/logging.hpp"

namespace thermite
{
    void start_event_loop();
}

#endif /* THERMITE_HPP */

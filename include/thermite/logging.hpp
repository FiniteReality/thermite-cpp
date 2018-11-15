#ifndef _THERMITE_LOGGING_HPP_
#define _THERMITE_LOGGING_HPP_

namespace thermite
{

template <typename... Ts>
constexpr void log(Ts&&... messages);

#if defined(THERMITE_DEBUG) && !defined(THERMITE_EXTERNAL_LOGGING)

}

// debug mode and external logging is disabled, log to stdout
#include <iostream>

namespace thermite
{

template <typename... Ts>
constexpr void log(Ts&&... messages)
{
    (std::cout << ... << std::forward<Ts>(messages)) << '\n' << std::flush;
}

#elif !defined(THERMITE_EXTERNAL_LOGGING)

// release mode and external logging is disabled, log nothing
template <typename... Ts>
constexpr void log(Ts&&...)
{
    // do nothing
}

#endif

}

#endif /* _THERMITE_LOGGING_HPP_ */

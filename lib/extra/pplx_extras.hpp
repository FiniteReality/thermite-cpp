#ifndef _PPLX_EXTRAS_HPP_
#define _PPLX_EXTRAS_HPP_

#include <chrono>
#include <exception>
#include <pplx/pplxtasks.h>

namespace pplx
{

namespace detail
{

pplx::task<std::chrono::milliseconds> _wait_for(
    const std::chrono::milliseconds& ms,
    const pplx::cancellation_token& cancelToken
        = pplx::cancellation_token::none());

}

const pplx::task<void> completed_task();

template <typename Rep, typename Period>
pplx::task<std::chrono::milliseconds> wait_for(
    const std::chrono::duration<Rep, Period>& duration,
    const pplx::cancellation_token& cancelToken
        = pplx::cancellation_token::none())
{
    return detail::_wait_for(
            std::chrono::duration_cast<std::chrono::milliseconds>(duration),
            cancelToken);
}

template <typename Clock, typename Duration = typename Clock::duration>
pplx::task<std::chrono::milliseconds> wait_until(
    const std::chrono::time_point<Clock, Duration>& time,
    const pplx::cancellation_token& cancelToken
        = pplx::cancellation_token::none())
{
    return wait_for(time - Clock::now(), cancelToken);
}

}

#endif /* _PPLX_EXTRAS_HPP_ */

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;

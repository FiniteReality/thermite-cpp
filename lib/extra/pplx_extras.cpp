#include "pplx_extras.hpp"

#include <boost/asio/steady_timer.hpp>
#include <pplx/threadpool.h>

// TODO: use Windows concurrency impl instead on windows platforms

pplx::task<std::chrono::milliseconds> pplx::detail::_wait_for(
    const std::chrono::milliseconds& time,
    const pplx::cancellation_token& cancelToken)
{
    pplx::task_completion_event<std::chrono::milliseconds> promise;

    auto timer = std::make_shared<boost::asio::steady_timer>(
        crossplat::threadpool::shared_instance().service());

    timer->expires_from_now(time);
    timer->async_wait([timer, promise](const boost::system::error_code & error)
    {
        if (error == boost::asio::error::operation_aborted)
        {
            if (!promise._IsTriggered())
                promise._Cancel();
        }
        else
        {
            // Return the extra amount of time waited for
            auto extra = std::chrono::steady_clock::now()
                - timer->expires_at();
            promise.set(
                std::chrono::duration_cast<std::chrono::milliseconds>(extra));
        }
    });

    auto result = pplx::create_task(promise);

    if (cancelToken.is_cancelable())
    {
        auto callback = cancelToken.register_callback([timer, promise]
        {
            timer->cancel();

            if (!promise._IsTriggered())
                promise._Cancel();
        });

        result.then([cancelToken, callback](
            pplx::task<std::chrono::milliseconds>)
        {
            cancelToken.deregister_callback(callback);
        });
    }

    result.then([timer](pplx::task<std::chrono::milliseconds>) {});

    return result;
}

const pplx::task<void> pplx::completed_task()
{
    static pplx::task_completion_event<void> promise;

    if (!promise._IsTriggered())
        promise.set();

    return pplx::create_task(promise);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;

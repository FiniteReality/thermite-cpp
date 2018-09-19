#include <boost/asio.hpp>
#include <pplx/threadpool.h>

namespace thermite::udp::detail
{

class udp_client_impl
{
public:
    udp_client_impl() noexcept
        : _service{crossplat::threadpool::shared_instance().service()},
        _socket{_service}
    { }

    ~udp_client_impl() = default;

    udp_client_impl(const udp_client_impl&) = default;
    udp_client_impl& operator=(
    const udp_client_impl&) = default;
    udp_client_impl(udp_client_impl&&) = default;
    udp_client_impl& operator=(udp_client_impl &&) = default;

    pplx::task<void> connect(const std::string& host, uint_least16_t port)
    {
        pplx::task_completion_event<void> promise;

        auto endpoint = boost::asio::ip::udp::endpoint{
            boost::asio::ip::address::from_string(host), port
        };

        _socket.async_connect(endpoint,
            [promise](const boost::system::error_code& error)
            {
                if (!promise._IsTriggered())
                {
                    if (error)
                    {
                        // TODO: convert these to std::error_code
                        promise.set_exception(std::system_error(
                            error.value(), std::system_category(),
                            error.message()));
                    }
                    else
                    {
                        promise.set();
                    }
                }
            });

        return pplx::create_task(promise)
            .then([promise]{});
    }

    pplx::task<std::size_t> send(const std::vector<uint8_t>& bytes)
    {
        pplx::task_completion_event<std::size_t> promise;

        _socket.async_send(boost::asio::buffer(bytes, bytes.size()),
            [promise](const boost::system::error_code& error,
                std::size_t transferred)
            {
                if (!promise._IsTriggered())
                {
                    if (error)
                    {
                        // TODO: convert these to std::error_code
                        promise.set_exception(std::system_error(
                            error.value(), std::system_category(),
                            error.message()));
                    }
                    else
                    {
                        promise.set(transferred);
                    }
                }
            });

        return pplx::create_task(promise)
            .then([promise](std::size_t t) { return t; });
    }

    pplx::task<std::vector<uint8_t>> receive(std::size_t max)
    {
        pplx::task_completion_event<std::size_t> promise;

        std::vector<uint8_t> bytes(max);

        _socket.async_receive(boost::asio::buffer(bytes, bytes.size()),
            [promise](const boost::system::error_code& error,
                        std::size_t transferred)
            {
                if (!promise._IsTriggered())
                {
                    if (error)
                    {
                        // TODO: convert these to std::error_code
                        promise.set_exception(std::system_error(
                            error.value(), std::system_category(),
                            error.message()));
                    }
                    else
                    {
                        promise.set(transferred);
                    }
                }
            });

        return pplx::create_task(promise)
            .then([promise, bytes](std::size_t t) mutable
            {
                bytes.resize(t);
                return bytes;
            });
    }

    pplx::task<std::vector<uint8_t>> receive_from(const std::string& host,
        uint_least16_t port,
        std::size_t max)
    {
        pplx::task_completion_event<std::size_t> promise;

        uint8_t* bytes = new uint8_t[max];

        auto endpoint = boost::asio::ip::udp::endpoint{
            boost::asio::ip::address::from_string(host), port
        };

        _socket.async_receive_from(boost::asio::buffer(bytes, max),
            endpoint,
            [promise](const boost::system::error_code& error,
                        std::size_t transferred)
            {
                if (!promise._IsTriggered())
                {
                    if (error)
                    {
                        // TODO: convert these to std::error_code
                        promise.set_exception(std::system_error(
                            error.value(), std::system_category(),
                            error.message()));
                    }
                    else
                    {
                        promise.set(transferred);
                    }
                }
            });

        return pplx::create_task(promise)
            .then([endpoint, promise, bytes, max](std::size_t t)
            {
                std::vector<uint8_t> result{bytes, bytes + max};
                result.resize(t);

                return result;
            });
    }

    void close()
    {
        _socket.close();
    }

private:
    boost::asio::io_service& _service;
    boost::asio::ip::udp::socket _socket;
};

}

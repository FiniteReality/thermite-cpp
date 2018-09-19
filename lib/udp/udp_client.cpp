#include <thermite/udp/udp_client.hpp>

namespace lib = thermite::udp;

// TODO: choose which impl to include based on platform
// TODO: use appropriate impl on Windows

#include "udp_client_asio.cpp"

lib::udp_client::udp_client() noexcept
    : _impl{std::make_shared<detail::udp_client_impl>()}
{ }


pplx::task<void> lib::udp_client::connect(const std::string& host,
    uint_least16_t port)
{
    return _impl->connect(host, port);
}

pplx::task<std::size_t> lib::udp_client::send(
    std::vector<uint8_t> bytes)
{
    return _impl->send(bytes);
}

pplx::task<std::vector<uint8_t>> lib::udp_client::receive(
    std::size_t max)
{
    return _impl->receive(max);
}

pplx::task<std::vector<uint8_t>> lib::udp_client::receive_from(
    const std::string& host,
    uint_least16_t port,
    std::size_t max)
{
    return _impl->receive_from(host, port, max);
}

void lib::udp_client::close()
{
    return _impl->close();
}

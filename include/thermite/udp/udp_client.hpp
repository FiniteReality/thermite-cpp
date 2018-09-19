#ifndef _UDP_CALLBACK_CLIENT_HPP_
#define _UDP_CALLBACK_CLIENT_HPP_

#include <memory>
#include <pplx/pplxtasks.h>

namespace thermite::udp
{

namespace detail
{
class udp_client_impl;
}


class udp_client
{
public:
    udp_client() noexcept;

    ~udp_client() = default;

    udp_client(const udp_client&) = default;
    udp_client& operator=(const udp_client&) = default;
    udp_client(udp_client&&) = default;
    udp_client& operator=(udp_client &&) = default;

    pplx::task<void> connect(const std::string& host, uint_least16_t port);

    pplx::task<std::size_t> send(std::vector<uint8_t> bytes);
    pplx::task<std::vector<uint8_t>> receive(std::size_t max);
    pplx::task<std::vector<uint8_t>> receive_from(const std::string& host,
        uint_least16_t port,
        std::size_t max);

    void close();

private:
    std::shared_ptr<detail::udp_client_impl> _impl;
};

}

#endif /* _UDP_CALLBACK_CLIENT_HPP_ */

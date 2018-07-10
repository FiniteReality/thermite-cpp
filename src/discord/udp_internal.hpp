#ifndef UDP_INTERNAL_HPP
#define UDP_INTERNAL_HPP

#include <uv.h>
#include <vector>

namespace thermite
{
namespace discord
{
namespace detail
{

class renter
{
public:
    explicit renter(size_t size) noexcept;

    renter(const renter& other) = delete;
    renter& operator=(const renter& other) = delete;

    renter(renter&& other) noexcept;

    renter& operator=(renter&& other) noexcept;

    ~renter() noexcept;

    std::vector<unsigned char>& buffer();

private:
    std::vector<unsigned char> _buffer;
    bool _valid;
};

class udpSendData
{
public:
    explicit udpSendData(renter&& rent) noexcept;
    ~udpSendData() = default;

    size_t num_bufs();

    uv_buf_t* bufs();

    uv_udp_send_t* uv();

private:
    renter _rent;
    uv_buf_t _buf;
    uv_udp_send_t _send;
};

}
}
}


#endif /* UDP_INTERNAL_HPP */

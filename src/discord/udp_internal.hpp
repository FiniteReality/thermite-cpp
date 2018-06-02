#ifndef _udp_internal_hpp_
#define _udp_internal_hpp_

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


#endif /* _udp_internal_hpp_ */

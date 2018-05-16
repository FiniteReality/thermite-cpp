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
    renter(size_t size);

    renter(const renter& other) = delete;
    renter& operator=(const renter& other) = delete;

    renter(renter&& other);

    renter& operator=(renter&& other);

    ~renter();

    std::vector<unsigned char> useBuffer(std::vector<unsigned char>&& buffer);

    std::vector<unsigned char>& buffer();

private:
    std::vector<unsigned char> _buffer;
    bool _valid;
};

class udpSendData
{
public:
    udpSendData(renter&& rent);

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

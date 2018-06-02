#include "udp_internal.hpp"

#include <queue>

using namespace thermite::discord;

static std::queue<std::vector<unsigned char>> buffers;

static std::vector<unsigned char> rentBuffer(size_t size)
{
    if (buffers.empty())
    {
        return std::vector<unsigned char>(size);
    }

    std::vector<unsigned char> buffer{std::move(buffers.front())};

    buffers.pop();

    if (buffer.size() < size)
        buffer.resize(size);

    return buffer;
}

static void returnBuffer(std::vector<unsigned char>&& buffer)
{
    buffer.clear();
    buffers.push(std::move(buffer));
}

detail::renter::renter(size_t size) noexcept
    : _buffer{rentBuffer(size)}, _valid{true}
{ }

detail::renter::renter(renter&& other) noexcept
    : _buffer{other._buffer}, _valid{true}
{
    other._valid = false;
}

detail::renter& detail::renter::operator=(renter&& other) noexcept
{
    if (this != &other)
    {
        other._valid = false;
        _buffer = std::move(other._buffer);
        _valid = true;
    }

    return *this;
}

detail::renter::~renter() noexcept
{
    if (_valid)
    {
        returnBuffer(std::move(_buffer));
    }
}

std::vector<unsigned char>& detail::renter::buffer()
{
    return _buffer;
}

detail::udpSendData::udpSendData(detail::renter&& rent) noexcept
    : _rent{std::move(rent)}, _buf{}, _send{}
{
    char* uv_data = reinterpret_cast<char*>(_rent.buffer().data());
    _buf = uv_buf_init(uv_data, _rent.buffer().size());
    _send.data = this;
}

size_t detail::udpSendData::num_bufs()
{
    return 1;
}

uv_buf_t* detail::udpSendData::bufs()
{
    return &_buf;
}

uv_udp_send_t* detail::udpSendData::uv()
{
    return &_send;
}

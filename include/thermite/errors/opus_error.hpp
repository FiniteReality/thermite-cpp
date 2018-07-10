#ifndef OPUS_ERROR_HPP
#define OPUS_ERROR_HPP

#include <stdexcept>

namespace thermite
{

class opus_error : public std::exception
{
public:
    explicit opus_error(int code) noexcept;

    opus_error(const opus_error&) noexcept = default;
    opus_error& operator=(const opus_error&) noexcept = default;

    virtual ~opus_error() = default;

    int code() const noexcept;
    virtual const char* what() const noexcept override;

private:
    int _code;
};

}

#endif /* OPUS_ERROR_HPP */

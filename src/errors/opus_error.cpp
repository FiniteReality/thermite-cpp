#include <thermite/errors/opus_error.hpp>

#include <opus/opus.h>

using namespace thermite;

opus_error::opus_error(int code) noexcept
    : _code{code}
{ }

int opus_error::code() const noexcept
{
    return _code;
}

const char* opus_error::what() const noexcept
{
    switch (_code)
    {
        case OPUS_BAD_ARG:
            return "bad argument";
        case OPUS_BUFFER_TOO_SMALL:
            return "buffer too small";
        case OPUS_INTERNAL_ERROR:
            return "internal error";
        case OPUS_INVALID_PACKET:
            return "invalid packet";
        case OPUS_UNIMPLEMENTED:
            return "unimplemented";
        case OPUS_INVALID_STATE:
            return "invalid state";
        case OPUS_ALLOC_FAIL:
            return "allocation failure";
        case OPUS_OK:
        default:
            return "unknown opus error";
    }
}

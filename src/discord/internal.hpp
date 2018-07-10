#ifndef INTERNAL_HPP
#define INTERNAL_HPP

#include <uWS/uWS.h>

namespace thermite
{
namespace discord
{

class voice_client;

namespace detail
{

void connect(std::string uri,
                discord::voice_client* client);

}
}
}

#endif /* INTERNAL_HPP */

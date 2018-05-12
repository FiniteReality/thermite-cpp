#ifndef INTERNAL_HPP
#define INTERNAL_HPP

#include <thermite.hpp>

#include <thermite/discord/voice_opcode.hpp>
#include <thermite/voice_client.hpp>

#include <uWS/uWS.h>

namespace thermite
{
namespace discord
{
namespace detail
{

void connect(std::string uri,
                discord::voice_client* client);

uv_loop_t* getUvLoop();

}

#ifdef THERMITE_DEBUG

#include <iostream>

std::ostream& operator<<(std::ostream& lhs, const voice_opcode& rhs);

#endif /* THERMITE_DEBUG */
}
}

#endif /* INTERNAL_HPP */

#ifndef DEBUG_HPP
#define DEBUG_HPP
#ifdef THERMITE_DEBUG

#include <iostream>
#include <uv.h>

#define DEBUG_LOG(expr) \
    std::cout << expr << std::endl


namespace thermite
{
namespace discord
{

enum class voice_opcode;

std::ostream& operator<<(std::ostream& lhs,
    const voice_opcode& rhs);

}

namespace detail
{

uv_loop_t* getUvLoop();

}
}

#else

#define DEBUG_LOG(expr)

#endif /* THERMITE_DEBUG */

#endif /* DEBUG_HPP */


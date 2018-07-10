#ifdef THERMITE_DEBUG

#include "debug.hpp"

std::ostream& thermite::discord::operator<<(
    std::ostream& lhs,
    const voice_opcode& rhs)
{
    return lhs << static_cast<int>(rhs);
}

#endif /* THERMITE_DEBUG */

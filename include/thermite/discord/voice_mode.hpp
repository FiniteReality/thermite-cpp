#ifndef VOICE_MODE_HPP
#define VOICE_MODE_HPP

namespace thermite
{
namespace discord
{

enum class voice_mode
{
    Unknown,
    XSalsa20_Poly1305,
    XSalsa20_Poly1305_Suffix,
    XSalsa20_Poly1305_Lite
};

}
}

#endif /* VOICE_MODE_HPP */

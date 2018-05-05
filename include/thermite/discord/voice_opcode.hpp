#ifndef VOICE_OPCODE_HPP
#define VOICE_OPCODE_HPP

namespace thermite
{
namespace discord
{

enum class voice_opcode
{
    Identify,
    SelectProtocol,
    Ready,
    Heartbeat,
    SessionDescription,
    Speaking,
    HeartbeatAck,
    ResumeConnection,
    Hello,
    Resumed
};

}
}

#endif /* VOICE_OPCODE_HPP */

#ifndef _VOICE_OPCODE_HPP_
#define _VOICE_OPCODE_HPP_

namespace thermite::discord
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

#endif /* _VOICE_OPCODE_HPP_ */

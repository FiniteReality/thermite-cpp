#ifndef _PLAYER_MANAGER_HPP_
#define _PLAYER_MANAGER_HPP_

#include <pplx/pplxtasks.h>
#include <string>

#include "player.hpp"

namespace thermite
{

class player_manager
{
public:
    explicit player_manager(std::string user_id) noexcept;


    pplx::task<player&> create_player(std::string endpoint,
        std::string guild_id, std::string session, std::string token);
};

}

#endif /* _PLAYER_MANAGER_HPP_ */

#include "pstream.h"

#include <thermite/discord/voice_client.hpp>

pplx::task<int> main_async(thermite::discord::voice_client client)
{
    pplx::task_completion_event<void> promise;

    return client.start()
        .then([promise]{
            return pplx::create_task(promise);
        })
        .then([client]() mutable
        {
            return client.stop();
        })
        .then([client, promise]
        {
            return 0;
        });
}

int main()
{
    redi::ipstream bot("python3 ./bot.py");

    std::cout << "getting info..." << std::endl;

    std::string guild_id; std::getline(bot, guild_id);
    std::string user_id; std::getline(bot, user_id);
    std::string endpoint; std::getline(bot, endpoint);
    std::string session; std::getline(bot, session);
    std::string token; std::getline(bot, token);

    std::cout
        << "===== Information from bot =====\n"
        << "guild id:   " << guild_id << '\n'
        << "user id:    " << user_id << '\n'
        << "endpoint:   " << endpoint << '\n'
        << "session id: " << session << '\n'
        << "token:      " << token << '\n'
        << std::endl;

    if (token.size() == 0)
        return 1;

    thermite::discord::voice_client client
    {
        guild_id, user_id, endpoint, session, token
    };

    try
    {
        return main_async(client).get();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        bot.close(); // ensure the process gets stopped
        throw;
    }
}

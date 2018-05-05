configure_file(bot.py.in bot.py COPYONLY)

add_executable(cli main.cpp)
target_link_libraries(cli thermite)

configure_file(bot.py.in bot.py COPYONLY)
configure_file(Night_Dancer.raw.in Night_Dancer.raw COPYONLY)

add_executable(cli main.cpp)
target_compile_options(cli PUBLIC -pthread)
target_link_libraries(cli PUBLIC thermite opus -pthread)
target_include_directories(cli PUBLIC thermite)

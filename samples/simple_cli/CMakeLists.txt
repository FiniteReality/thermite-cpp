set(SAMPLE_SRCS main.cpp)

add_executable(simple_cli ${SAMPLE_SRCS})
target_link_libraries(simple_cli PUBLIC thermite)
target_include_directories(simple_cli PUBLIC thermite)

configure_file(bot.py bot.py COPYONLY)

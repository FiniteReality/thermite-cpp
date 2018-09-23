find_package(Opus REQUIRED)

set(SAMPLE_SRCS main.cpp)

add_executable(simple_cli ${SAMPLE_SRCS})
target_compile_options(simple_cli PUBLIC
    -pthread)
target_link_libraries(simple_cli PUBLIC
    thermite
    ${Opus_LIBRARIES}
    -pthread)
target_include_directories(simple_cli PUBLIC
    thermite
    ${Opus_INCLUDE_DIRS})

configure_file(bot.py bot.py COPYONLY)

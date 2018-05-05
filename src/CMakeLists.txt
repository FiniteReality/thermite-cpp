if(THERMITE_DEBUG)
    message(STATUS "Building thermite with debug logging enabled")
    add_definitions(-DTHERMITE_DEBUG)
endif(THERMITE_DEBUG)

set(THERMITE_SRCS
discord/voice_client.cpp discord/opcode.cpp discord/heartbeat.cpp
discord/udp.cpp

socket_thread.cpp util.cpp)

add_library(thermite SHARED ${THERMITE_SRCS})
add_dependencies(thermite uWebSockets)

target_link_libraries(thermite PUBLIC ${LIBUV_LIBRARY} ${uWS_LIBRARIES})
target_include_directories(thermite PUBLIC ../include)

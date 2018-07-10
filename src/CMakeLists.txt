find_package(uv REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(TBB REQUIRED)
find_package(uWS REQUIRED)
find_package(sodium REQUIRED)

if(THERMITE_DEBUG)
    message(STATUS "Building thermite with debug logging enabled")
    add_definitions(-DTHERMITE_DEBUG)
endif(THERMITE_DEBUG)

set(THERMITE_SRCS
discord/voice_client.cpp discord/ws_events.cpp discord/protocol.cpp
discord/udp.cpp discord/udp_buffer.cpp

debug.cpp socket_hub.cpp)

add_library(thermite SHARED ${THERMITE_SRCS})
add_dependencies(thermite uWS)
target_compile_options(thermite BEFORE PUBLIC
    -DUSE_LIBUV
    -DRAPIDJSON_HAS_STDSTRING=1)

target_link_libraries(thermite PUBLIC
    ${uWS_LIBRARIES}
    ${TBB_LIBRARIES}
    ${sodium_LIBRARIES})
target_include_directories(thermite PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${RapidJSON_INCLUDE_DIRS}
    ${uv_INCLUDE_DIRS}
    ${uvw_INCLUDE_DIRS}
    ${uWS_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}
    ${sodium_INCLUDE_DIRS})

set_property(TARGET thermite PROPERTY CXX_STANDARD 14)
set_property(TARGET thermite PROPERTY CXX_STANDARD_REQUIRED ON)

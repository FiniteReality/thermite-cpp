find_package(Boost 1.62 REQUIRED
    COMPONENTS system
    OPTIONAL_COMPONENTS regex)
find_package(OpenSSL REQUIRED)
find_package(TBB REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(sodium REQUIRED)

include(TestBigEndian)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})


if(THERMITE_DEBUG)
    message(STATUS "Building thermite with debug logging enabled")
    add_definitions(-DTHERMITE_DEBUG)
endif(THERMITE_DEBUG)

# Support big endian platforms
TEST_BIG_ENDIAN(_big_endian)
if(_big_endian)
    add_definitions(-DTHERMITE_BIG_ENDIAN)
endif(_big_endian)
mark_as_advanced(_big_endian)


set(THERMITE_SRCS
# extras
extra/pplx_extras.cpp

# udp implementation
udp/udp_client.cpp

# core lib
discord/voice_client.cpp discord/voice_client_events.cpp)


add_library(thermite SHARED ${THERMITE_SRCS})
target_link_libraries(thermite PUBLIC
    ${TBB_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OpenSSL_LIBRARIES}
    ${cpprestsdk_LIBRARIES}
    ${sodium_LIBRARIES})
target_include_directories(thermite PUBLIC
    ${RapidJSON_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${OpenSSL_INCLUDE_DIRS}
    ${cpprestsdk_INCLUDE_DIRS}
    ${sodium_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
# Disable casablanca defining the U macro, stomping other uses of it
target_compile_definitions(thermite PUBLIC
    -D_TURN_OFF_PLATFORM_STRING
    -DTURN_OFF_PLATFORM_STRING)


set_property(TARGET thermite PROPERTY CXX_STANDARD 17)
set_property(TARGET thermite PROPERTY CXX_STANDARD_REQUIRED ON)

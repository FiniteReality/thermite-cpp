cmake_minimum_required(VERSION 3.0)

set(PATCHES_DIRECTORY "${CMAKE_SOURCE_DIR}/patches")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

project(thermite)

OPTION(THERMITE_DEBUG "Enable debug logging" ON)
OPTION(THERMITE_BUILD_SAMPLES "Build samples" ON)

find_package(libuv REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(TBB REQUIRED)
find_package(uWS REQUIRED)

include_directories(${LIBUV_INCLUDE_DIR} ${uWS_INCLUDE_DIR} include)

add_subdirectory(src)

if(THERMITE_BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

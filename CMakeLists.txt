cmake_minimum_required(VERSION 3.11)

project(thermite)

option(THERMITE_DEBUG "Enable debug logging" ON)
option(THERMITE_EXTERNAL_LOGGING "Enable external logging" OFF)
option(THERMITE_ENABLE_ASAN "Enable -fsanitize=address" OFF)
option(THERMITE_BUILD_SAMPLES "Build samples" ON)


if(THERMITE_ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_STATIC_LINKER_FLAGS
        "${CMAKE_STATIC_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
endif()

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/cmake/modules"
)


add_subdirectory(lib)

if(THERMITE_BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

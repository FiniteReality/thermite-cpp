find_package(uv REQUIRED)

include(ExternalProject)
include(FindPackageHandleStandardArgs)

set(uWS_PREFIX "${CMAKE_BINARY_DIR}/thirdparty/uWebSockets")
set(uWS_INCLUDE_DIR ${uWS_PREFIX}/include)
set(uWS_LIBRARY ${uWS_PREFIX}/lib/libuWS.so)

set(uWS_CFLAGS "-DUSE_LIBUV -UUSE_EPOLL")

# TODO: support windows build (uses VC++.vcxproj rather than Makefile)
ExternalProject_Add(uWS
    PREFIX ${uWS_PREFIX}
    GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git

    CONFIGURE_COMMAND ""
    UPDATE_COMMAND git pull -r

    BUILD_COMMAND $(MAKE) CFLAGS=${uWS_CFLAGS} ${CMAKE_SYSTEM_NAME}
    BUILD_IN_SOURCE 1

    INSTALL_COMMAND $(MAKE) install PREFIX=../..

    TEST_BEFORE_INSTALL 0
    TEST_AFTER_INSTALL 0
)

find_package_handle_standard_args(uWS
    DEFAULT_MSG
    uWS_INCLUDE_DIR
    uWS_LIBRARY
)

# Hide internal variables
mark_as_advanced(uWS_PREFIX uWS_INCLUDE_DIR uWS_LIBRARY uWS_CFLAGS)

# Set standard variables
if(uWS_FOUND)
    set(uWS_INCLUDE_DIRS ${uWS_INCLUDE_DIR} ${uv_INCLUDE_DIRS})
    set(uWS_LIBRARIES ${uWS_LIBRARY} ssl crypto z ${uv_LIBRARIES})
endif()

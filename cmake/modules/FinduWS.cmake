include(ExternalProject)
include(FindPackageHandleStandardArgs)

set(uWS_PREFIX "${CMAKE_BINARY_DIR}/thirdparty/uWebSockets")

file(GLOB uWS_PATCHES ${PATCHES_DIRECTORY}/uWebSockets/*.patch)

ExternalProject_Add(uWebSockets
    PREFIX ${uWS_PREFIX}
    GIT_REPOSITORY https://github.com/uNetworking/uWebSockets

    CONFIGURE_COMMAND true
    UPDATE_COMMAND true

    PATCH_COMMAND git checkout af50492212005775fb3d55b0f77ade4723414533
    COMMAND git am ${uWS_PATCHES}

    BUILD_COMMAND $(MAKE) CFLAGS="-D USE_LIBUV -U USE_EPOLL"
    BUILD_IN_SOURCE 1

    INSTALL_COMMAND $(MAKE) install PREFIX=../..

    TEST_BEFORE_INSTALL 0
    TEST_AFTER_INSTALL 0
)

mark_as_advanced(uWS_PREFIX uWS_PATCHES)

set(uWS_INCLUDE_DIR ${uWS_PREFIX}/include)
set(uWS_LIBRARIES ${uWS_PREFIX}/lib/libuWS.so ssl crypto z)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(uWebSockets
    DEFAULT_MSG
    uWS_INCLUDE_DIR
    uWS_LIBRARIES
)

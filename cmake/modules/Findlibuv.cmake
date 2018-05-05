include(FindPackageHandleStandardArgs)

find_path(LIBUV_INCLUDE_DIR NAMES uv.h)
find_library(LIBUV_LIBRARY NAMES uv libuv)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBUV
    REQUIRED_VARS
    LIBUV_INCLUDE_DIR
    LIBUV_LIBRARY
)

# Hide internal variables
mark_as_advanced(LIBUV_INCLUDE_DIR LIBUV_LIBRARY)

# Set standard variables
if(LIBUV_FOUND)
    set(LIBUV_INCLUDE_DIRS ${LIBUV_INCLUDE_DIR})
    set(LIBUV_LIBRARIES ${LIBUV_LIBRARY})
endif()

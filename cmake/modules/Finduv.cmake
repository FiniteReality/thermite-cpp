include(FindPackageHandleStandardArgs)

find_path(uv_INCLUDE_DIR NAMES uv.h)
find_library(uv_LIBRARY NAMES uv libuv)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(uv
    REQUIRED_VARS
    uv_INCLUDE_DIR
    uv_LIBRARY
)

# Hide internal variables
mark_as_advanced(uv_INCLUDE_DIR uv_LIBRARY)

# Set standard variables
if(uv_FOUND)
    set(uv_INCLUDE_DIRS ${uv_INCLUDE_DIR})
    set(uv_LIBRARIES ${uv_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)

find_path(sodium_INCLUDE_DIR NAMES sodium.h)
find_library(sodium_LIBRARY NAMES sodium)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(sodium
    REQUIRED_VARS
    sodium_INCLUDE_DIR
    sodium_LIBRARY
)

# Hide internal variables
mark_as_advanced(sodium_INCLUDE_DIR sodium_LIBRARY)

# Set standard variables
if(sodium_FOUND)
    set(sodium_INCLUDE_DIRS ${sodium_INCLUDE_DIR})
    set(sodium_LIBRARIES ${sodium_LIBRARY})
endif()

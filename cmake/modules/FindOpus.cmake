include(FindPackageHandleStandardArgs)

find_path(Opus_INCLUDE_DIR NAMES opus/opus.h)
find_library(Opus_LIBRARY NAMES opus)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Opus
    REQUIRED_VARS
    Opus_INCLUDE_DIR
    Opus_LIBRARY
)

# Hide internal variables
mark_as_advanced(Opus_INCLUDE_DIR Opus_LIBRARY)

# Set standard variables
if (Opus_FOUND)
    set(Opus_INCLUDE_DIRS ${Opus_INCLUDE_DIR})
    set(Opus_LIBRARIES ${Opus_LIBRARY})
endif()

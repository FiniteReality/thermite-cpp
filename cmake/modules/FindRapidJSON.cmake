include(ExternalProject)
include(FindPackageHandleStandardArgs)

find_path(RapidJSON_INCLUDE_DIR NAMES rapidjson/rapidjson.h)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(RapidJSON
    DEFAULT_MSG
    RapidJSON_INCLUDE_DIR
)

mark_as_advanced(RapidJSON_INCLUDE_DIR)

if(RapidJSON_FOUND)
	set(RapidJSON_INCLUDE_DIRS ${RapidJSON_INCLUDE_DIR})
endif()

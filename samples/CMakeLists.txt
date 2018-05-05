set(THERMITE_SAMPLES cli)

foreach(sample IN LISTS THERMITE_SAMPLES)
    option(THERMITE_BUILD_SAMPLE_${sample} "Build thermite sample ${sample}" ON)

    if(THERMITE_BUILD_SAMPLE_${sample})
        message(STATUS "Building sample: ${sample}")
        add_subdirectory(${sample})
    endif()
endforeach()

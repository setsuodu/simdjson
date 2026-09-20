cmake_minimum_required(VERSION 3.15)
project(simdjson_unity VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# simdjson single-header + Unity C wrapper
add_library(simdjson_unity SHARED
    simdjson_unity_wrapper.cpp
    simdjson.cpp
)

target_include_directories(simdjson_unity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Platform-specific naming (Unity Plugins conventions)
if(WIN32)
    set_target_properties(simdjson_unity PROPERTIES
        OUTPUT_NAME "simdjson_unity"
        PREFIX ""
        SUFFIX ".dll"
    )
elseif(APPLE)
    set_target_properties(simdjson_unity PROPERTIES
        OUTPUT_NAME "simdjson_unity"
        PREFIX "lib"
        SUFFIX ".dylib"
    )
else()
    # Linux / Android / others
    set_target_properties(simdjson_unity PROPERTIES
        OUTPUT_NAME "simdjson_unity"
        PREFIX "lib"
        SUFFIX ".so"
    )
endif()

# Default to Release if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

target_compile_options(simdjson_unity PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/O2 /EHsc>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O3>
)

# Hide symbols by default on ELF (wrapper already marks exports)
if(NOT WIN32 AND NOT APPLE)
    target_compile_options(simdjson_unity PRIVATE -fvisibility=hidden)
endif()

install(TARGETS simdjson_unity
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
)

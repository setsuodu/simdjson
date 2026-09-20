cmake_minimum_required(VERSION 3.15)
project(simdjson_unity VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# simdjson single-header
add_library(simdjson_unity SHARED
    simdjson_unity_wrapper.cpp
    simdjson.cpp
)

target_include_directories(simdjson_unity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Platform specific
if(WIN32)
    set_target_properties(simdjson_unity PROPERTIES
        OUTPUT_NAME "simdjson_unity"
        PREFIX ""
    )
elseif(APPLE)
    set_target_properties(simdjson_unity PROPERTIES
        OUTPUT_NAME "simdjson_unity"
        SUFFIX ".dylib"
    )
else()
    set_target_properties(simdjson_unity PROPERTIES
        OUTPUT_NAME "simdjson_unity"
        PREFIX "lib"
    )
endif()

# Optimize
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

target_compile_options(simdjson_unity PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/O2>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O3>
)

# Note: simdjson uses exceptions in some paths; enable if needed
# For production you may want -fexceptions

install(TARGETS simdjson_unity
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
)

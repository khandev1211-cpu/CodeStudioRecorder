cmake_minimum_required(VERSION 3.10)
project(codestudio_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(codestudio_engine SHARED
    src/codestudio_engine.cpp
)

if(WIN32)
    set_target_properties(codestudio_engine PROPERTIES
        PREFIX ""
        SUFFIX ".dll"
    )
endif()

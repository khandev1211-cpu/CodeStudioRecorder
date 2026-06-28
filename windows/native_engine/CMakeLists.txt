cmake_minimum_required(VERSION 3.10)
project(codestudio_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(codestudio_engine SHARED
    src/codestudio_engine.cpp
    src/recording_engine.cpp
    src/wgc_capturer.cpp
    src/wasapi_audio_engine.cpp
    src/texture_pool.cpp
    src/ffmpeg_encoder.cpp
)

if(WIN32)
    set_target_properties(codestudio_engine PROPERTIES
        PREFIX ""
        SUFFIX ".dll"
    )
    # Link to Windows APIs
    target_link_libraries(codestudio_engine PRIVATE
        windowsapp
        dwmapi
        d3d11
        dxgi
    )
endif()

# Ensure the DLL is copied to the same directory as the executable
install(TARGETS codestudio_engine RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}" COMPONENT Runtime)

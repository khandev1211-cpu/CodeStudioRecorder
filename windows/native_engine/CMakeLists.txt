cmake_minimum_required(VERSION 3.10)
project(codestudio_engine)

# Use C++20 for better WinRT and Coroutine support
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(FFMPEG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/ffmpeg")
if(EXISTS "${FFMPEG_DIR}/include/libavcodec/avcodec.h")
    message(STATUS "FFmpeg headers found. Building with real encoder.")
    add_definitions(-DUSE_REAL_FFMPEG)
    include_directories(${FFMPEG_DIR}/include)
    link_directories(${FFMPEG_DIR}/lib/x64)
    set(FFMPEG_LIBS avcodec avformat avutil swscale swresample)
else()
    message(WARNING "FFmpeg headers not found. Building with Mock Encoder fallback.")
endif()

add_library(codestudio_engine SHARED
    src/codestudio_engine.cpp
    src/recording_engine.cpp
    src/wgc_capturer.cpp
    src/wasapi_audio_engine.cpp
    src/texture_pool.cpp
    src/ffmpeg_encoder.cpp
    src/window_utils.cpp
    src/settings_manager.cpp
    src/encoder_factory.cpp
    src/audio_mixer.cpp
    src/cs_logger.cpp
)

if(WIN32)
    set_target_properties(codestudio_engine PROPERTIES
        PREFIX ""
        SUFFIX ".dll"
    )

    target_link_libraries(codestudio_engine PRIVATE
        windowsapp
        dwmapi
        d3d11
        dxgi
        ${FFMPEG_LIBS}
    )

    # Suppress deprecation and experimental feature warnings
    target_compile_definitions(codestudio_engine PRIVATE
        _SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS
        _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

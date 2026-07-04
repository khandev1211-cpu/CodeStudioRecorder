cmake_minimum_required(VERSION 3.10)
project(codestudio_engine)

# Use C++20 for best WinRT and Coroutine support
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(FFMPEG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/ffmpeg")
if(EXISTS "${FFMPEG_DIR}/include/libavcodec/avcodec.h")
    message(STATUS "FFmpeg headers found. Building with real encoder.")
    add_definitions(-DUSE_REAL_FFMPEG)
    include_directories(${FFMPEG_DIR}/include)
    link_directories(${FFMPEG_DIR}/lib ${FFMPEG_DIR}/lib/x64)
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
    src/cursor_highlight_processor.cpp
    src/click_animation_processor.cpp
    src/annotation_processor.cpp
    src/zoom_processor.cpp
    src/webcam_processor.cpp
    src/db_manager.cpp
    src/global_mouse_hook.cpp
    src/audio_utils.cpp
    src/webcam_utils.cpp
    src/plugin_manager.cpp
    src/ai_processor.cpp
    src/caption_engine.cpp
    src/system_check.cpp
    src/overlay_manager.cpp
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
        d2d1
        mfplat
        mf
        mfreadwrite
        mfuuid
        ${FFMPEG_LIBS}
    )

    target_compile_definitions(codestudio_engine PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
        _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
        _SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

install(TARGETS codestudio_engine RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}" COMPONENT Runtime)

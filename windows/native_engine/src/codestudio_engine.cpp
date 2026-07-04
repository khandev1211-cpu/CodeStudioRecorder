#include <cctype>
#include <cwctype>
#include <iostream>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <map>
#include <set>
#include <filesystem>

// Project headers
#include "codestudio_engine.h"
#include "recording_engine.h"
#include "window_utils.h"
#include "audio_utils.h"
#include "webcam_utils.h"
#include "system_check.h"
#include "settings_manager.h"
#include "encoder_factory.h"
#include "cs_logger.h"

// System headers last
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

class SessionManager {
public:
    static SessionManager& instance() {
        static SessionManager inst;
        return inst;
    }

    SessionManager() {
        char* appData;
        size_t len;
        _dupenv_s(&appData, &len, "LOCALAPPDATA");
        std::string logPath = "codestudio.log";
        if (appData) {
            std::string dirPath = std::string(appData) + "\\CodeStudioRecorder";
            logPath = dirPath + "\\codestudio.log";
            try {
                std::filesystem::create_directories(std::filesystem::path(dirPath));
            } catch (...) {}
            free(appData);
        }

        cs::Logger::instance().setLogFile(logPath);
        CS_LOG_INFO("Session Manager Initialized. Log: " + logPath);
    }

    int32_t beginSession(const cs::RecordingConfig& config) {
        try {
            return engine_.start(config);
        } catch (const std::exception& e) {
            CS_LOG_ERR("Exception in beginSession: " + std::string(e.what()));
            return -100;
        } catch (...) {
            CS_LOG_ERR("Unknown exception in beginSession");
            return -101;
        }
    }

    int32_t endSession() {
        return engine_.stop();
    }

    int32_t pauseSession() {
        return engine_.pause();
    }

    int32_t resumeSession() {
        return engine_.resume();
    }

    void getStats(cs::RecordingStats* stats) {
        engine_.getStats(stats);
    }

    void getAudioLevels(float* mic, float* sys) {
        engine_.getAudioLevels(mic, sys);
    }

    const char* getNextCaption() {
        static cs::CaptionSegment segment;
        if (engine_.getNextCaption(segment)) {
            return segment.text.c_str();
        }
        return nullptr;
    }

    cs::RecordingStatus getStatus() const {
        return engine_.getStatus();
    }

    void setProcessorEnabled(int32_t index, bool enabled) {
        engine_.setProcessorEnabled(index, enabled);
    }

    void setPluginEnabled(int32_t index, bool enabled) {
        engine_.setPluginEnabled(index, enabled);
    }

    int32_t getPluginCount() const {
        return engine_.getPluginCount();
    }

    void handleMouseClick(float x, float y) {
        engine_.handleMouseClick(x, y);
    }

    void addAnnotation(int32_t type, float x1, float y1, float x2, float y2, uint32_t color, float width) {
        engine_.addAnnotation(type, x1, y1, x2, y2, color, width);
    }

    void clearAnnotations() {
        engine_.clearAnnotations();
    }

    void undoAnnotation() {
        engine_.undoAnnotation();
    }

    void setZoomLevel(float level) {
        engine_.setZoomLevel(level);
    }

    void setWebcamPosition(float x, float y, float width, float height) {
        engine_.setWebcamPosition(x, y, width, height);
    }

    void addChapterMarker(const char* label) {
        engine_.addChapterMarker(label ? label : "Marker");
    }

private:
    cs::RecordingEngine engine_;
};

extern "C" {

CSE_API int32_t cse_start_recording(cs::RecordingConfig* config) {
    CS_LOG_INFO("cse_start_recording called");
    if (!config) {
        CS_LOG_ERR("cse_start_recording: config is null");
        return -1;
    }
    CS_LOG_INFO("Config: " + std::to_string(config->width) + "x" + std::to_string(config->height) + " @ " + std::to_string(config->fps) + "fps");
    return SessionManager::instance().beginSession(*config);
}

CSE_API int32_t cse_stop_recording() {
    return SessionManager::instance().endSession();
}

CSE_API int32_t cse_pause_recording() {
    return SessionManager::instance().pauseSession();
}

CSE_API int32_t cse_resume_recording() {
    return SessionManager::instance().resumeSession();
}

CSE_API void cse_get_stats(cs::RecordingStats* stats) {
    if (stats) {
        SessionManager::instance().getStats(stats);
    }
}

CSE_API void cse_get_audio_levels(float* mic, float* system) {
    SessionManager::instance().getAudioLevels(mic, system);
}

CSE_API const char* cse_get_next_caption() {
    return SessionManager::instance().getNextCaption();
}

CSE_API int32_t cse_get_status() {
    return static_cast<int32_t>(SessionManager::instance().getStatus());
}

CSE_API void cse_enumerate_windows(WindowCallback callback) {
    auto windows = cs::WindowUtils::enumerateWindows();
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    for (const auto& win : windows) {
        std::string title = converter.to_bytes(win.title);
        NativeWindowInfo nwi;
        nwi.hwnd = reinterpret_cast<int64_t>(win.hwnd);
        nwi.title = title.c_str();
        callback(&nwi);
    }
}

CSE_API void cse_enumerate_audio_devices(bool capture, AudioDeviceCallback callback) {
    auto devices = cs::AudioUtils::enumerateDevices(capture);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    for (const auto& dev : devices) {
        std::string id = converter.to_bytes(dev.id);
        std::string name = converter.to_bytes(dev.name);
        NativeAudioDeviceInfo nadi;
        nadi.id = id.c_str();
        nadi.name = name.c_str();
        nadi.is_default = dev.is_default;
        callback(&nadi);
    }
}

CSE_API void cse_enumerate_webcams(WebcamCallback callback) {
    auto cameras = cs::WebcamUtils::enumerateCameras();
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    for (const auto& cam : cameras) {
        std::string id = converter.to_bytes(cam.id);
        std::string name = converter.to_bytes(cam.name);
        NativeWebcamDeviceInfo nwdi;
        nwdi.id = id.c_str();
        nwdi.name = name.c_str();
        callback(&nwdi);
    }
}

CSE_API void cse_set_setting_string(const char* key, const char* value) {
    cs::SettingsManager::instance().setString(key, value);
    cs::SettingsManager::instance().save();
}

CSE_API const char* cse_get_setting_string(const char* key, const char* default_value) {
    static std::string last_result;
    last_result = cs::SettingsManager::instance().getString(key, default_value);
    return last_result.c_str();
}

CSE_API void cse_set_setting_int(const char* key, int32_t value) {
    cs::SettingsManager::instance().setInt(key, value);
    cs::SettingsManager::instance().save();
}

CSE_API int32_t cse_get_setting_int(const char* key, int32_t default_value) {
    return cs::SettingsManager::instance().getInt(key, default_value);
}

CSE_API void cse_set_processor_enabled(int32_t index, bool enabled) {
    SessionManager::instance().setProcessorEnabled(index, enabled);
}

CSE_API void cse_set_plugin_enabled(int32_t index, bool enabled) {
    SessionManager::instance().setPluginEnabled(index, enabled);
}

CSE_API int32_t cse_get_plugin_count() {
    return SessionManager::instance().getPluginCount();
}

CSE_API void cse_report_mouse_click(float x, float y) {
    SessionManager::instance().handleMouseClick(x, y);
}

CSE_API void cse_add_annotation(int32_t type, float x1, float y1, float x2, float y2, uint32_t color, float width) {
    SessionManager::instance().addAnnotation(type, x1, y1, x2, y2, color, width);
}

CSE_API void cse_clear_annotations() {
    SessionManager::instance().clearAnnotations();
}

CSE_API void cse_undo_annotation() {
    SessionManager::instance().undoAnnotation();
}

CSE_API void cse_set_zoom_level(float level) {
    SessionManager::instance().setZoomLevel(level);
}

CSE_API void cse_set_webcam_position(float x, float y, float width, float height) {
    SessionManager::instance().setWebcamPosition(x, y, width, height);
}

CSE_API void cse_add_chapter_marker(const char* label) {
    SessionManager::instance().addChapterMarker(label);
}

CSE_API int32_t cse_generate_thumbnail(const char* video_path, const char* thumb_path) {
    if (!video_path || !thumb_path) return -1;
    auto encoder = cs::EncoderFactory::createEncoder();
    return encoder->generateThumbnail(video_path, thumb_path) ? 0 : -2;
}

CSE_API bool cse_check_system_requirements() {
    auto results = cs::SystemCheck::runFullCheck();
    for (const auto& res : results) {
        if (!res.met) return false;
    }
    return true;
}

} // extern "C"

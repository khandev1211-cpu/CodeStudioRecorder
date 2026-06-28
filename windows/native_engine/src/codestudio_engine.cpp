#include "codestudio_engine.h"
#include "recording_engine.h"
#include "window_utils.h"
#include "settings_manager.h"
#include "cs_logger.h"
#include <codecvt>
#include <locale>

class SessionManager {
public:
    static SessionManager& instance() {
        static SessionManager inst;
        return inst;
    }

    SessionManager() {
        cs::Logger::instance().setLogFile("codestudio.log");
        CS_LOG_INFO("Session Manager Initialized");
    }

    int32_t beginSession(const cs::RecordingConfig& config) {
        return engine_.start(config);
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

    cs::RecordingStatus getStatus() const {
        return engine_.getStatus();
    }

private:
    cs::RecordingEngine engine_;
};

CSE_API int32_t cse_start_recording(cs::RecordingConfig* config) {
    if (!config) {
        CS_LOG_ERR("cse_start_recording: config is null");
        return -1;
    }
    CS_LOG_INFO("FFI: cse_start_recording");
    return SessionManager::instance().beginSession(*config);
}

CSE_API int32_t cse_stop_recording() {
    CS_LOG_INFO("FFI: cse_stop_recording");
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
        callback(nwi);
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

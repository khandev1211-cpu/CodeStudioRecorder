#include "codestudio_engine.h"
#include "recording_engine.h"

class SessionManager {
public:
    static SessionManager& instance() {
        static SessionManager inst;
        return inst;
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
    if (!config) return -1;
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

CSE_API int32_t cse_get_status() {
    return static_cast<int32_t>(SessionManager::instance().getStatus());
}

#include "codestudio_engine.h"
#include <iostream>

class SessionManager {
public:
    static SessionManager& instance() {
        static SessionManager inst;
        return inst;
    }

    int32_t beginSession(const cs::RecordingConfig& config) {
        std::cout << "Starting recording to: " << config.output_path << std::endl;
        status_ = cs::RecordingStatus::Recording;
        return 0; // Success
    }

    int32_t endSession() {
        std::cout << "Stopping recording" << std::endl;
        status_ = cs::RecordingStatus::Completed;
        return 0;
    }

    int32_t pauseSession() {
        status_ = cs::RecordingStatus::Paused;
        return 0;
    }

    int32_t resumeSession() {
        status_ = cs::RecordingStatus::Recording;
        return 0;
    }

    void getStats(cs::RecordingStats* stats) {
        stats->elapsed_ms = 1000; // Mock data
        stats->dropped_frames = 0;
        stats->encoder_load = 0.1f;
    }

    cs::RecordingStatus getStatus() const {
        return status_;
    }

private:
    cs::RecordingStatus status_ = cs::RecordingStatus::Idle;
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

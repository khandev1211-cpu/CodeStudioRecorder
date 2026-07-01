#pragma once
#include <cstdint>

namespace cs {

enum class RecordingStatus : int32_t {
    Idle = 0,
    Initializing = 1,
    Ready = 2,
    Recording = 3,
    Paused = 4,
    Flushing = 5,
    Finalizing = 6,
    Completed = 7,
    Error = 8
};

struct RecordingConfig {
    uint32_t width;
    uint32_t height;
    uint32_t fps;
    const char* output_path;
    bool capture_cursor;
    bool capture_audio;
    int64_t target_hwnd; // 0 for full screen
    const char* encoder_name; // e.g. "h264_nvenc", "libx264"
    const char* mic_device_id;
    const char* sys_audio_device_id;
    const char* webcam_device_id;

    // AI Toggles
    bool ai_noise_removal;
    bool ai_auto_captions;
    bool ai_silence_detection;
};

struct RecordingStats {
    int64_t elapsed_ms;
    uint32_t dropped_frames;
    float encoder_load;
};

struct ChapterMarker {
    int64_t timestamp_ms;
    const char* label;
};

struct AudioDeviceInfo {
    const char* id;
    const char* name;
    bool is_default;
};

} // namespace cs

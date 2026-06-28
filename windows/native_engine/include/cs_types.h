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
};

struct RecordingStats {
    int64_t elapsed_ms;
    uint32_t dropped_frames;
    float encoder_load;
};

} // namespace cs

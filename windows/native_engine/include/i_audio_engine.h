#pragma once
#include "cs_types.h"
#include <functional>
#include <vector>

namespace cs {

struct AudioBuffer {
    const float* samples;
    uint32_t frame_count;
    uint32_t channels;
    uint32_t sample_rate;
    int64_t timestamp_qpc;
};

using AudioCallback = std::function<void(const AudioBuffer&)>;

class IAudioEngine {
public:
    virtual ~IAudioEngine() = default;
    virtual bool initialize(const RecordingConfig& config) = 0;
    virtual void start(AudioCallback callback) = 0;
    virtual void stop() = 0;
};

} // namespace cs

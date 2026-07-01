#pragma once
#include "i_audio_engine.h"
#include <vector>
#include <atomic>
#include <mutex>

namespace cs {

class AISilenceDetector {
public:
    AISilenceDetector();

    // Returns true if speech is detected in the buffer
    bool process(const AudioBuffer& buffer);

    void setThreshold(float threshold) { threshold_ = threshold; }
    bool isSilent() const { return is_silent_; }

private:
    std::atomic<float> threshold_{ 0.01f };
    std::atomic<bool> is_silent_{ true };
    int64_t silence_counter_ = 0;
};

class AINoiseSuppressor {
public:
    AINoiseSuppressor();

    // Processes audio in-place
    void process(AudioBuffer& buffer);

    void setEnabled(bool enabled) { enabled_ = enabled; }

private:
    std::atomic<bool> enabled_{ false };
    // Placeholder for RNNoise/DeepFilterNet state
};

} // namespace cs

#include "ai_processor.h"
#include <cmath>
#include <algorithm>

namespace cs {

AISilenceDetector::AISilenceDetector() {}

bool AISilenceDetector::process(const AudioBuffer& buffer) {
    float peak = 0.0f;
    size_t count = buffer.frame_count * buffer.channels;

    for (size_t i = 0; i < count; i++) {
        peak = std::max(peak, std::abs(buffer.samples[i]));
    }

    bool currently_silent = peak < threshold_;

    if (currently_silent) {
        silence_counter_++;
    } else {
        silence_counter_ = 0;
    }

    // Require ~200ms of silence to trigger (assuming 48kHz, ~10ms buffers)
    is_silent_ = silence_counter_ > 20;

    return !is_silent_;
}

AINoiseSuppressor::AINoiseSuppressor() {}

void AINoiseSuppressor::process(AudioBuffer& buffer) {
    if (!enabled_) return;

    // TODO: Integrate RNNoise or DeepFilterNet.
    // For now, implement a simple soft-gate as a placeholder.
    float threshold = 0.005f;
    size_t count = buffer.frame_count * buffer.channels;

    for (size_t i = 0; i < count; i++) {
        if (std::abs(buffer.samples[i]) < threshold) {
            buffer.samples[i] *= 0.5f; // Reduce noise
        }
    }
}

} // namespace cs

#include "audio_mixer.h"
#include <algorithm>
#include <cmath>

namespace cs {

AudioMixer::AudioMixer() {}

void AudioMixer::pushMicBuffer(const AudioBuffer& buffer) {
    std::lock_guard<std::mutex> lock(mutex_);

    float peak = 0;
    size_t count = buffer.frame_count * buffer.channels;
    for (size_t i = 0; i < count; ++i) {
        peak = std::max(peak, std::abs(buffer.samples[i]));
    }
    last_mic_peak_ = peak;

    mic_queue_.insert(mic_queue_.end(), buffer.samples, buffer.samples + count);
}

void AudioMixer::pushSystemBuffer(const AudioBuffer& buffer) {
    std::lock_guard<std::mutex> lock(mutex_);

    float peak = 0;
    size_t count = buffer.frame_count * buffer.channels;
    for (size_t i = 0; i < count; ++i) {
        peak = std::max(peak, std::abs(buffer.samples[i]));
    }
    last_system_peak_ = peak;

    system_queue_.insert(system_queue_.end(), buffer.samples, buffer.samples + count);
}

bool AudioMixer::getNextMixedBuffer(std::vector<float>& output, uint32_t& channels, uint32_t& sample_rate) {
    // ... (rest of the method stays same)
}

void AudioMixer::getLevels(float* mic_level, float* system_level) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (mic_level) *mic_level = last_mic_peak_;
    if (system_level) *system_level = last_system_peak_;

    // Decay peaks slightly for smoother UI if not updated
    last_mic_peak_ *= 0.8f;
    last_system_peak_ *= 0.8f;
}
    std::lock_guard<std::mutex> lock(mutex_);

    size_t samples_to_mix = std::min(mic_queue_.size(), system_queue_.size());
    if (samples_to_mix == 0) {
        // If one is empty, we might still want to output the other
        if (!mic_queue_.empty()) {
            output = std::move(mic_queue_);
            mic_queue_.clear();
            channels = target_channels_;
            sample_rate = target_sample_rate_;
            return true;
        }
        if (!system_queue_.empty()) {
            output = std::move(system_queue_);
            system_queue_.clear();
            channels = target_channels_;
            sample_rate = target_sample_rate_;
            return true;
        }
        return false;
    }

    output.resize(samples_to_mix);
    for (size_t i = 0; i < samples_to_mix; ++i) {
        // Simple additive mixing with clamping
        float mixed = mic_queue_[i] + system_queue_[i];
        output[i] = std::clamp(mixed, -1.0f, 1.0f);
    }

    mic_queue_.erase(mic_queue_.begin(), mic_queue_.begin() + samples_to_mix);
    system_queue_.erase(system_queue_.begin(), system_queue_.begin() + samples_to_mix);

    channels = target_channels_;
    sample_rate = target_sample_rate_;
    return true;
}

} // namespace cs

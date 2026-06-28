#pragma once
#include <vector>
#include <cstdint>
#include <mutex>
#include "i_audio_engine.h"

namespace cs {

class AudioMixer {
public:
    AudioMixer();
    ~AudioMixer() = default;

    void pushMicBuffer(const AudioBuffer& buffer);
    void pushSystemBuffer(const AudioBuffer& buffer);

    // Mixes available buffers into a single output buffer
    // Returns true if a full mix was produced
    bool getNextMixedBuffer(std::vector<float>& output, uint32_t& channels, uint32_t& sample_rate);

private:
    std::vector<float> mic_queue_;
    std::vector<float> system_queue_;
    std::mutex mutex_;

    uint32_t target_sample_rate_ = 48000;
    uint32_t target_channels_ = 2;
};

} // namespace cs

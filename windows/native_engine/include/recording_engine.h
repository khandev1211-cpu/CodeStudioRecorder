#pragma once
#include "cs_types.h"
#include "i_capturer.h"
#include "i_audio_engine.h"
#include "i_encoder.h"
#include <memory>
#include <atomic>
#include <mutex>

namespace cs {

class RecordingEngine {
public:
    RecordingEngine();
    ~RecordingEngine();

    int32_t start(const RecordingConfig& config);
    int32_t stop();
    int32_t pause();
    int32_t resume();

    void getStats(RecordingStats* stats);
    RecordingStatus getStatus() const;

private:
    std::unique_ptr<ICapturer> capturer_;
    std::unique_ptr<IAudioEngine> audio_engine_;
    std::unique_ptr<IEncoder> encoder_;

    std::atomic<RecordingStatus> status_{RecordingStatus::Idle};
    mutable std::mutex stats_mutex_;
    RecordingStats stats_{};

    void onVideoFrame(const VideoFrame& frame);
    void onAudioBuffer(const AudioBuffer& buffer);
};

} // namespace cs

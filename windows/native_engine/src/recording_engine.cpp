#include "recording_engine.h"
#include "mock_engines.h"
#include "wasapi_audio_engine.h"
#include "wgc_capturer.h"
#include "encoder_factory.h"

namespace cs {

RecordingEngine::RecordingEngine() {
    capturer_ = std::make_unique<WGCCapturer>();
    audio_engine_ = std::make_unique<WASAPIAudioEngine>();
    encoder_ = EncoderFactory::createEncoder();
}

RecordingEngine::~RecordingEngine() {
    stop();
}

int32_t RecordingEngine::start(const RecordingConfig& config) {
    if (status_ != RecordingStatus::Idle && status_ != RecordingStatus::Completed) {
        return -1;
    }

    status_ = RecordingStatus::Initializing;

    if (!encoder_->initialize(config) ||
        !capturer_->initialize(config) ||
        !audio_engine_->initialize(config)) {
        status_ = RecordingStatus::Error;
        return -2;
    }

    status_ = RecordingStatus::Recording;

    capturer_->start([this](const VideoFrame& frame) { onVideoFrame(frame); });
    audio_engine_->start([this](const AudioBuffer& buffer) { onAudioBuffer(buffer); });

    return 0;
}

int32_t RecordingEngine::stop() {
    if (status_ != RecordingStatus::Recording && status_ != RecordingStatus::Paused) {
        return 0;
    }

    status_ = RecordingStatus::Flushing;

    capturer_->stop();
    audio_engine_->stop();
    encoder_->finalize();

    status_ = RecordingStatus::Completed;
    return 0;
}

int32_t RecordingEngine::pause() {
    if (status_ == RecordingStatus::Recording) {
        capturer_->pause();
        status_ = RecordingStatus::Paused;
    }
    return 0;
}

int32_t RecordingEngine::resume() {
    if (status_ == RecordingStatus::Paused) {
        capturer_->resume();
        status_ = RecordingStatus::Recording;
    }
    return 0;
}

void RecordingEngine::getStats(RecordingStats* stats) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    *stats = stats_;
}

RecordingStatus RecordingEngine::getStatus() const {
    return status_;
}

void RecordingEngine::onVideoFrame(const VideoFrame& frame) {
    if (status_ == RecordingStatus::Recording) {
        encoder_->encodeVideoFrame(frame);

        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.elapsed_ms += 16; // Simplified mock
    }
}

void RecordingEngine::onAudioBuffer(const AudioBuffer& buffer) {
    if (status_ == RecordingStatus::Recording) {
        encoder_->encodeAudioBuffer(buffer);
    }
}

} // namespace cs

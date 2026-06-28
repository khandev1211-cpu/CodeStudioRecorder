#include "recording_engine.h"
#include "mock_engines.h"
#include "wasapi_audio_engine.h"
#include "wgc_capturer.h"
#include "encoder_factory.h"
#include "cs_logger.h"
#include <chrono>

namespace cs {

RecordingEngine::RecordingEngine() {
    CS_LOG_INFO("Initializing Recording Engine");
    capturer_ = std::make_unique<WGCCapturer>();
    mic_engine_ = std::make_unique<WASAPIAudioEngine>(WASAPIAudioEngine::DeviceMode::Capture);
    system_audio_engine_ = std::make_unique<WASAPIAudioEngine>(WASAPIAudioEngine::DeviceMode::Loopback);
    encoder_ = EncoderFactory::createEncoder();
    mixer_ = std::make_unique<AudioMixer>();
}

RecordingEngine::~RecordingEngine() {
    stop();
}

int32_t RecordingEngine::start(const RecordingConfig& config) {
    if (status_ != RecordingStatus::Idle && status_ != RecordingStatus::Completed) {
        CS_LOG_WARN("Engine start requested while not idle");
        return -1;
    }

    CS_LOG_INFO("Starting recording session...");
    status_ = RecordingStatus::Initializing;

    if (!encoder_->initialize(config)) {
        CS_LOG_ERR("Failed to initialize encoder");
        status_ = RecordingStatus::Error;
        return -2;
    }

    if (!capturer_->initialize(config)) {
        CS_LOG_ERR("Failed to initialize capturer");
        status_ = RecordingStatus::Error;
        return -3;
    }

    // Optional audio initialization
    if (config.capture_audio) {
        if (!mic_engine_->initialize(config) || !system_audio_engine_->initialize(config)) {
            CS_LOG_WARN("Failed to initialize audio, continuing without it");
        }
    }

    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_ = {};
        start_time_ = std::chrono::steady_clock::now();
    }

    status_ = RecordingStatus::Recording;

    capturer_->start([this](const VideoFrame& frame) { onVideoFrame(frame); });

    if (config.capture_audio) {
        mic_engine_->start([this](const AudioBuffer& buffer) { onMicBuffer(buffer); });
        system_audio_engine_->start([this](const AudioBuffer& buffer) { onSystemBuffer(buffer); });
    }

    CS_LOG_INFO("Recording started successfully");
    return 0;
}

int32_t RecordingEngine::stop() {
    if (status_ != RecordingStatus::Recording && status_ != RecordingStatus::Paused) {
        return 0;
    }

    status_ = RecordingStatus::Flushing;

    capturer_->stop();
    mic_engine_->stop();
    system_audio_engine_->stop();
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
    if (status_ == RecordingStatus::Recording || status_ == RecordingStatus::Paused) {
        auto now = std::chrono::steady_clock::now();
        stats_.elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
    }
    *stats = stats_;
}

RecordingStatus RecordingEngine::getStatus() const {
    return status_;
}

void RecordingEngine::onVideoFrame(const VideoFrame& frame) {
    if (status_ == RecordingStatus::Recording) {
        encoder_->encodeVideoFrame(frame);

        std::lock_guard<std::mutex> lock(stats_mutex_);
        auto now = std::chrono::steady_clock::now();
        stats_.elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
    }
}

void RecordingEngine::onMicBuffer(const AudioBuffer& buffer) {
    if (status_ == RecordingStatus::Recording) {
        mixer_->pushMicBuffer(buffer);

        std::vector<float> mixed;
        uint32_t channels, sample_rate;
        if (mixer_->getNextMixedBuffer(mixed, channels, sample_rate)) {
            AudioBuffer mixed_buffer;
            mixed_buffer.samples = mixed.data();
            mixed_buffer.frame_count = static_cast<uint32_t>(mixed.size() / channels);
            mixed_buffer.channels = channels;
            mixed_buffer.sample_rate = sample_rate;
            mixed_buffer.timestamp_qpc = buffer.timestamp_qpc;
            encoder_->encodeAudioBuffer(mixed_buffer);
        }
    }
}

void RecordingEngine::onSystemBuffer(const AudioBuffer& buffer) {
    if (status_ == RecordingStatus::Recording) {
        mixer_->pushSystemBuffer(buffer);
    }
}

} // namespace cs

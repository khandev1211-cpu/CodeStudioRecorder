#include <chrono>
#include <vector>
#include <memory>
#include <mutex>
#include <string>

#include "recording_engine.h"
#include "mock_engines.h"
#include "wasapi_audio_engine.h"
#include "wgc_capturer.h"
#include "encoder_factory.h"
#include "cursor_highlight_processor.h"
#include "click_animation_processor.h"
#include "annotation_processor.h"
#include "zoom_processor.h"
#include "webcam_processor.h"
#include "cs_logger.h"

namespace cs {

RecordingEngine::RecordingEngine() {
    CS_LOG_INFO("Initializing Recording Engine");

    capturer_ = std::make_unique<WGCCapturer>();
    mic_engine_ = std::make_unique<WASAPIAudioEngine>(WASAPIAudioEngine::DeviceMode::Capture);
    system_audio_engine_ = std::make_unique<WASAPIAudioEngine>(WASAPIAudioEngine::DeviceMode::Loopback);

    encoder_ = EncoderFactory::createEncoder();
    mixer_ = std::make_unique<AudioMixer>();

    processors_.push_back(std::make_unique<CursorHighlightProcessor>()); // 0
    processors_.push_back(std::make_unique<ClickAnimationProcessor>());   // 1
    processors_.push_back(std::make_unique<AnnotationProcessor>());       // 2
    processors_.push_back(std::make_unique<ZoomProcessor>());             // 3
    processors_.push_back(std::make_unique<WebcamProcessor>());           // 4
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
        status_ = RecordingStatus::Idle;
        return -2;
    }

    if (!capturer_->initialize(config)) {
        CS_LOG_ERR("Failed to initialize capturer");
        encoder_->finalize();
        status_ = RecordingStatus::Idle;
        return -3;
    }

    bool mic_ok = false;
    bool sys_ok = false;
    if (config.capture_audio) {
        mic_ok = mic_engine_->initialize(config);
        sys_ok = system_audio_engine_->initialize(config);
        if (!mic_ok || !sys_ok) {
            CS_LOG_WARN("Failed to initialize audio, some streams may be missing");
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
        if (mic_ok) mic_engine_->start([this](const AudioBuffer& buffer) { onMicBuffer(buffer); });
        if (sys_ok) system_audio_engine_->start([this](const AudioBuffer& buffer) { onSystemBuffer(buffer); });
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

void RecordingEngine::setProcessorEnabled(int32_t index, bool enabled) {
    if (index >= 0 && index < (int32_t)processors_.size()) {
        processors_[index]->setEnabled(enabled);
    }
}

void RecordingEngine::handleMouseClick(float x, float y) {
    if (processors_.size() > 1) {
        auto* clickProc = static_cast<ClickAnimationProcessor*>(processors_[1].get());
        clickProc->addClick(x, y);
    }
}

void RecordingEngine::addAnnotation(int32_t type, float x1, float y1, float x2, float y2, uint32_t color, float width) {
    if (processors_.size() > 2) {
        auto* annProc = static_cast<AnnotationProcessor*>(processors_[2].get());
        AnnotationShape shape;
        shape.type = static_cast<AnnotationType>(type);
        shape.x1 = x1;
        shape.y1 = y1;
        shape.x2 = x2;
        shape.y2 = y2;
        shape.color = color;
        shape.stroke_width = width;
        annProc->addShape(shape);
    }
}

void RecordingEngine::clearAnnotations() {
    if (processors_.size() > 2) {
        static_cast<AnnotationProcessor*>(processors_[2].get())->clear();
    }
}

void RecordingEngine::undoAnnotation() {
    if (processors_.size() > 2) {
        static_cast<AnnotationProcessor*>(processors_[2].get())->undo();
    }
}

void RecordingEngine::setZoomLevel(float level) {
    if (processors_.size() > 3) {
        static_cast<ZoomProcessor*>(processors_[3].get())->setZoomLevel(level);
    }
}

void RecordingEngine::setWebcamPosition(float x, float y, float width, float height) {
    if (processors_.size() > 4) {
        static_cast<WebcamProcessor*>(processors_[4].get())->setPosition(x, y, width, height);
    }
}

void RecordingEngine::onVideoFrame(const VideoFrame& frame) {
    if (status_ == RecordingStatus::Recording) {
        VideoFrame processedFrame = frame;
        for (auto& processor : processors_) {
            if (processor->isEnabled()) {
                processor->process(processedFrame, capturer_->getDevice(), capturer_->getContext());
            }
        }

        encoder_->encodeVideoFrame(processedFrame);

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

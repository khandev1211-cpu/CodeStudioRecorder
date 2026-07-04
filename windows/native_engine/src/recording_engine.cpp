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
#include "plugin_manager.h"
#include "overlay_manager.h"
#include "cs_logger.h"
#include "global_mouse_hook.h"

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

    noise_suppressor_ = std::make_unique<AINoiseSuppressor>();
    silence_detector_ = std::make_unique<AISilenceDetector>();
    caption_engine_ = std::make_unique<CaptionEngine>();

    worker_running_ = true;
    worker_thread_ = std::thread(&RecordingEngine::workerLoop, this);

    // Load external plugins
    PluginManager::instance().loadPlugins("plugins");
}

RecordingEngine::~RecordingEngine() {
    stop();
    worker_running_ = false;
    cv_.notify_all();
    if (worker_thread_.joinable()) worker_thread_.join();
}

int32_t RecordingEngine::start(const RecordingConfig& config) {
    if (status_ != RecordingStatus::Idle && status_ != RecordingStatus::Completed) {
        CS_LOG_WARN("Engine start requested while not idle");
        return -1;
    }

    {
        std::lock_guard<std::mutex> lock(markers_mutex_);
        session_markers_.clear();
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

    texture_pool_ = std::make_unique<TexturePool>(capturer_->getDevice(), size_t(10));

    bool mic_ok = false;
    bool sys_ok = false;
    if (config.capture_audio) {
        mic_ok = mic_engine_->initialize(config);
        sys_ok = system_audio_engine_->initialize(config);
        if (!mic_ok || !sys_ok) {
            CS_LOG_WARN("Failed to initialize audio, some streams may be missing");
        }
    }

    for (auto& processor : processors_) {
        processor->onStart(config);
    }

    for (auto& plugin : PluginManager::instance().getPlugins()) {
        plugin->onStart(config);
    }

    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_ = {};
        start_time_ = std::chrono::steady_clock::now();
    }

    status_ = RecordingStatus::Recording;

    OverlayManager::instance().start();
    OverlayManager::instance().setHighlightEnabled(config.capture_cursor);

    noise_suppressor_->setEnabled(config.ai_noise_removal);
    caption_engine_->setEnabled(config.ai_auto_captions);

    // Start Global Mouse Hook to capture clicks anywhere on the screen
    GlobalMouseHook::instance().start([this](float x, float y) {
        this->handleMouseClick(x, y);
    });

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

    CS_LOG_INFO("Stop requested. Finalizing in background...");
    status_ = RecordingStatus::Flushing;

    OverlayManager::instance().stop();
    GlobalMouseHook::instance().stop();
    capturer_->stop();
    mic_engine_->stop();
    system_audio_engine_->stop();

    EncoderTask task;
    task.type = EncoderTask::Type::Finalize;
    pushTask(std::move(task));

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

void RecordingEngine::getAudioLevels(float* mic_level, float* system_level) {
    if (mixer_) {
        mixer_->getLevels(mic_level, system_level);
    }
}

bool RecordingEngine::getNextCaption(CaptionSegment& caption) {
    if (caption_engine_) {
        return caption_engine_->getNextCaption(caption);
    }
    return false;
}

RecordingStatus RecordingEngine::getStatus() const {
    return status_;
}

void RecordingEngine::setProcessorEnabled(int32_t index, bool enabled) {
    if (index >= 0 && index < (int32_t)processors_.size()) {
        processors_[index]->setEnabled(enabled);
    }
}

void RecordingEngine::setPluginEnabled(int32_t index, bool enabled) {
    auto& plugins = PluginManager::instance().getPlugins();
    if (index >= 0 && index < (int32_t)plugins.size()) {
        plugins[index]->setEnabled(enabled);
    }
}

int32_t RecordingEngine::getPluginCount() const {
    return (int32_t)PluginManager::instance().getPlugins().size();
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
        OverlayManager::instance().updateAnnotations(annProc->getShapes());
    }
}

void RecordingEngine::clearAnnotations() {
    if (processors_.size() > 2) {
        auto* annProc = static_cast<AnnotationProcessor*>(processors_[2].get());
        annProc->clear();
        OverlayManager::instance().updateAnnotations(annProc->getShapes());
    }
}

void RecordingEngine::undoAnnotation() {
    if (processors_.size() > 2) {
        auto* annProc = static_cast<AnnotationProcessor*>(processors_[2].get());
        annProc->undo();
        OverlayManager::instance().updateAnnotations(annProc->getShapes());
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

void RecordingEngine::addChapterMarker(const std::string& label) {
    if (status_ != RecordingStatus::Recording) return;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();

    std::lock_guard<std::mutex> lock(markers_mutex_);
    session_markers_.push_back({ elapsed, label });
    CS_LOG_INFO("Added marker: " + label + " at " + std::to_string(elapsed) + "ms");
}

std::vector<ChapterMarker> RecordingEngine::getMarkers() const {
    std::lock_guard<std::mutex> lock(markers_mutex_);
    std::vector<ChapterMarker> result;
    for (const auto& m : session_markers_) {
        // Warning: pointers to strings in session_markers_ must stay valid
        // This is safe if called right before stop() finishes
        result.push_back({ m.timestamp_ms, m.label.c_str() });
    }
    return result;
}

void RecordingEngine::onVideoFrame(const VideoFrame& frame) {
    if (status_ == RecordingStatus::Recording) {
        POINT p;
        if (GetCursorPos(&p)) {
            OverlayManager::instance().updateCursorPosition((float)p.x, (float)p.y);
        }

        // Copy capture frame to a pool-managed texture that supports D2D drawing
        auto target_tex = texture_pool_->acquire(frame.width, frame.height, DXGI_FORMAT_B8G8R8A8_UNORM);
        if (!target_tex) return;

        capturer_->getContext()->CopyResource(target_tex.Get(), static_cast<ID3D11Texture2D*>(frame.data));

        VideoFrame processedFrame = frame;
        processedFrame.data = target_tex.Get();

        for (auto& processor : processors_) {
            if (processor->isEnabled()) {
                processor->process(processedFrame, capturer_->getDevice(), capturer_->getContext());
            }
        }

        for (auto& plugin : PluginManager::instance().getPlugins()) {
            if (plugin->isEnabled()) {
                plugin->process(processedFrame, capturer_->getDevice(), capturer_->getContext());
            }
        }

        // Push to encoding queue
        EncoderTask task;
        task.type = EncoderTask::Type::Video;
        task.video_frame = processedFrame;
        // The texture must be kept alive until encoded.
        // We'll increment the reference manually since VideoFrame uses raw void*.
        if (processedFrame.data) static_cast<IUnknown*>(processedFrame.data)->AddRef();

        pushTask(std::move(task));

        std::lock_guard<std::mutex> lock(stats_mutex_);
        auto now = std::chrono::steady_clock::now();
        stats_.elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
    }
}

void RecordingEngine::onMicBuffer(const AudioBuffer& buffer) {
    if (status_ == RecordingStatus::Recording) {
        AudioBuffer processed = buffer;
        // AI processing... (keep as is)
        noise_suppressor_->process(processed);
        silence_detector_->process(processed);
        caption_engine_->pushAudio(processed);

        mixer_->pushMicBuffer(processed);

        std::vector<float> mixed;
        uint32_t channels, sample_rate;
        if (mixer_->getNextMixedBuffer(mixed, channels, sample_rate)) {
            EncoderTask task;
            task.type = EncoderTask::Type::Audio;
            task.audio_samples = std::move(mixed);
            task.channels = channels;
            task.sample_rate = sample_rate;
            task.timestamp = buffer.timestamp_qpc;
            pushTask(std::move(task));
        }
    }
}

void RecordingEngine::onSystemBuffer(const AudioBuffer& buffer) {
    if (status_ == RecordingStatus::Recording) {
        mixer_->pushSystemBuffer(buffer);
    }
}

void RecordingEngine::pushTask(EncoderTask&& task) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    task_queue_.push(std::move(task));
    cv_.notify_one();
}

void RecordingEngine::workerLoop() {
    while (worker_running_) {
        EncoderTask task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this] { return !task_queue_.empty() || !worker_running_; });
            if (!worker_running_ && task_queue_.empty()) break;
            task = std::move(task_queue_.front());
            task_queue_.pop();
        }

        if (task.type == EncoderTask::Type::Video) {
            encoder_->encodeVideoFrame(task.video_frame);
            if (task.video_frame.data) {
                static_cast<IUnknown*>(task.video_frame.data)->Release();
                // Release back to pool if it was a pool texture
                // We'll rely on the pool's own internal ref counting or just raw ptr for now.
            }
        } else if (task.type == EncoderTask::Type::Audio) {
            AudioBuffer ab;
            ab.samples = task.audio_samples.data();
            ab.frame_count = (uint32_t)(task.audio_samples.size() / task.channels);
            ab.channels = task.channels;
            ab.sample_rate = task.sample_rate;
            ab.timestamp_qpc = task.timestamp;
            encoder_->encodeAudioBuffer(ab);
        } else if (task.type == EncoderTask::Type::Finalize) {
            encoder_->finalize();
            status_ = RecordingStatus::Completed;
        }
    }
}

} // namespace cs

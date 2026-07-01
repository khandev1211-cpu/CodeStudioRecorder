#pragma once
#include "cs_types.h"
#include "i_capturer.h"
#include "i_audio_engine.h"
#include "i_encoder.h"
#include "audio_mixer.h"
#include "i_frame_processor.h"
#include "ai_processor.h"
#include "caption_engine.h"
#include <memory>
#include <atomic>
#include <mutex>
#include <chrono>
#include <vector>

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
    void getAudioLevels(float* mic_level, float* system_level);
    bool getNextCaption(CaptionSegment& caption);
    RecordingStatus getStatus() const;
    void setProcessorEnabled(int32_t index, bool enabled);
    void setPluginEnabled(int32_t index, bool enabled);
    int32_t getPluginCount() const;
    void handleMouseClick(float x, float y);
    void addAnnotation(int32_t type, float x1, float y1, float x2, float y2, uint32_t color, float width);
    void clearAnnotations();
    void undoAnnotation();
    void setZoomLevel(float level);
    void setWebcamPosition(float x, float y, float width, float height);
    void addChapterMarker(const std::string& label);
    std::vector<ChapterMarker> getMarkers() const;

private:
    std::unique_ptr<ICapturer> capturer_;
    std::unique_ptr<IAudioEngine> mic_engine_;
    std::unique_ptr<IAudioEngine> system_audio_engine_;
    std::unique_ptr<IEncoder> encoder_;
    std::unique_ptr<AudioMixer> mixer_;
    std::vector<std::unique_ptr<IFrameProcessor>> processors_;

    std::unique_ptr<AINoiseSuppressor> noise_suppressor_;
    std::unique_ptr<AISilenceDetector> silence_detector_;
    std::unique_ptr<CaptionEngine> caption_engine_;

    struct MarkerInternal {
        int64_t timestamp_ms;
        std::string label;
    };
    std::vector<MarkerInternal> session_markers_;
    mutable std::mutex markers_mutex_;

    std::atomic<RecordingStatus> status_{RecordingStatus::Idle};
    mutable std::mutex stats_mutex_;
    RecordingStats stats_{};
    std::chrono::steady_clock::time_point start_time_;

    void onVideoFrame(const VideoFrame& frame);
    void onMicBuffer(const AudioBuffer& buffer);
    void onSystemBuffer(const AudioBuffer& buffer);
};

} // namespace cs

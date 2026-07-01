#pragma once
#include "i_audio_engine.h"
#include <vector>
#include <string>
#include <deque>
#include <mutex>

namespace cs {

struct CaptionSegment {
    int64_t start_ms;
    int64_t end_ms;
    std::string text;
};

class CaptionEngine {
public:
    CaptionEngine();
    ~CaptionEngine();

    void pushAudio(const AudioBuffer& buffer);
    bool getNextCaption(CaptionSegment& caption);

    void setEnabled(bool enabled) { enabled_ = enabled; }

private:
    bool enabled_ = false;
    std::deque<float> audio_buffer_;
    std::deque<CaptionSegment> ready_captions_;
    std::mutex mutex_;

    // Placeholder for local Whisper model (ONNX)
    void runInference();

    int64_t total_samples_processed_ = 0;
};

} // namespace cs

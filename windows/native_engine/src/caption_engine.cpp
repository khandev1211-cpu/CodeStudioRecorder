#include "caption_engine.h"
#include <algorithm>

namespace cs {

CaptionEngine::CaptionEngine() {}
CaptionEngine::~CaptionEngine() {}

void CaptionEngine::pushAudio(const AudioBuffer& buffer) {
    if (!enabled_) return;

    std::lock_guard<std::mutex> lock(mutex_);

    // Convert to mono if needed (Whisper expects mono)
    if (buffer.channels == 2) {
        for (size_t i = 0; i < buffer.frame_count; i++) {
            audio_buffer_.push_back((buffer.samples[i * 2] + buffer.samples[i * 2 + 1]) * 0.5f);
        }
    } else {
        audio_buffer_.insert(audio_buffer_.end(), buffer.samples, buffer.samples + buffer.frame_count);
    }

    total_samples_processed_ += buffer.frame_count;

    // Run inference every 3 seconds of audio (~144,000 samples at 48kHz)
    if (audio_buffer_.size() > 144000) {
        runInference();
    }
}

void CaptionEngine::runInference() {
    // This is where the local Whisper model (ONNX) would be called.
    // For now, we simulate detection.

    CaptionSegment segment;
    segment.start_ms = (total_samples_processed_ - 144000) * 1000 / 48000;
    segment.end_ms = total_samples_processed_ * 1000 / 48000;
    segment.text = "[AI Caption Placeholder: Local Whisper Inference]";

    ready_captions_.push_back(segment);

    // Keep some overlap for better context (e.g. 1 second)
    size_t samples_to_remove = 144000 - 48000;
    if (audio_buffer_.size() > samples_to_remove) {
        audio_buffer_.erase(audio_buffer_.begin(), audio_buffer_.begin() + samples_to_remove);
    }
}

bool CaptionEngine::getNextCaption(CaptionSegment& caption) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (ready_captions_.empty()) return false;

    caption = ready_captions_.front();
    ready_captions_.pop_front();
    return true;
}

} // namespace cs

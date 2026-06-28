#pragma once
#include "cs_types.h"
#include "i_capturer.h"
#include "i_audio_engine.h"

namespace cs {

class IEncoder {
public:
    virtual ~IEncoder() = default;
    virtual bool initialize(const RecordingConfig& config) = 0;
    virtual void encodeVideoFrame(const VideoFrame& frame) = 0;
    virtual void encodeAudioBuffer(const AudioBuffer& buffer) = 0;
    virtual void finalize() = 0;
};

} // namespace cs

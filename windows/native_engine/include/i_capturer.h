#pragma once
#include "cs_types.h"
#include <functional>

namespace cs {

struct VideoFrame {
    void* data; // Platform specific texture or buffer
    uint32_t width;
    uint32_t height;
    int64_t timestamp_qpc;
};

using FrameCallback = std::function<void(const VideoFrame&)>;

class ICapturer {
public:
    virtual ~ICapturer() = default;
    virtual bool initialize(const RecordingConfig& config) = 0;
    virtual void start(FrameCallback callback) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
};

} // namespace cs

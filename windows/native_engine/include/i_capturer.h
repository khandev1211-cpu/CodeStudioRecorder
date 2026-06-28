#pragma once
#include "cs_types.h"
#include <functional>

// Forward declarations
struct ID3D11Device;
struct ID3D11DeviceContext;

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

    virtual ID3D11Device* getDevice() = 0;
    virtual ID3D11DeviceContext* getContext() = 0;
};

} // namespace cs

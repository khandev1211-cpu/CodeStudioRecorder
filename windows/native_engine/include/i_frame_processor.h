#pragma once
#include "i_capturer.h"
#include <d3d11.h>

namespace cs {

class IFrameProcessor {
public:
    virtual ~IFrameProcessor() = default;

    // Called when recording starts
    virtual void onStart(const RecordingConfig& config) {}

    // Process the frame. Can modify the texture or metadata.
    // 'device' and 'context' are provided for D3D11 operations.
    virtual void process(VideoFrame& frame, ID3D11Device* device, ID3D11DeviceContext* context) = 0;

    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;

    virtual const char* getName() const { return "Unknown Plugin"; }
    virtual const char* getDescription() const { return "No description available."; }
    virtual const char* getAuthor() const { return "Anonymous"; }
    virtual const char* getVersion() const { return "1.0.0"; }
};

} // namespace cs

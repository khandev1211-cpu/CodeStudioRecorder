#pragma once
#include "i_frame_processor.h"
#include <wrl/client.h>
#include <d2d1_1.h>
#include <mutex>

namespace cs {

class ZoomProcessor : public IFrameProcessor {
public:
    ZoomProcessor();
    ~ZoomProcessor() = default;

    void onStart(const RecordingConfig& config) override {
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = config;
    }

    void process(VideoFrame& frame, ID3D11Device* device, ID3D11DeviceContext* context) override;

    bool isEnabled() const override { return enabled_; }
    void setEnabled(bool enabled) override { enabled_ = enabled; }

    void setZoomLevel(float level) {
        std::lock_guard<std::mutex> lock(mutex_);
        zoom_level_ = level;
    }

private:
    void initializeD2D(ID3D11Device* device);

    bool enabled_ = false;
    float zoom_level_ = 1.5f;
    float current_x_ = 0, current_y_ = 0; // Smoothed center
    RecordingConfig config_{};
    std::mutex mutex_;

    Microsoft::WRL::ComPtr<ID2D1Factory1> d2d_factory_;
    Microsoft::WRL::ComPtr<ID2D1Device> d2d_device_;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2d_context_;

    // Auxiliary texture for zooming
    Microsoft::WRL::ComPtr<ID3D11Texture2D> zoom_texture_;
};

} // namespace cs

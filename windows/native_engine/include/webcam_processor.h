#pragma once
#include "i_frame_processor.h"
#include <wrl/client.h>
#include <d2d1_1.h>
#include <mutex>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

namespace cs {

class WebcamProcessor : public IFrameProcessor {
public:
    WebcamProcessor();
    ~WebcamProcessor();

    void onStart(const RecordingConfig& config) override {
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = config;
    }

    void process(VideoFrame& frame, ID3D11Device* device, ID3D11DeviceContext* context) override;

    bool isEnabled() const override { return enabled_; }
    void setEnabled(bool enabled) override;

    void setPosition(float x, float y, float width, float height) {
        std::lock_guard<std::mutex> lock(mutex_);
        rect_ = D2D1::RectF(x, y, x + width, y + height);
    }

private:
    void initializeD2D(ID3D11Device* device);
    void startCamera();
    void stopCamera();

    bool enabled_ = false;
    RecordingConfig config_{};
    D2D1_RECT_F rect_ = D2D1::RectF(20, 20, 340, 200); // Default PiP position
    std::mutex mutex_;

    Microsoft::WRL::ComPtr<ID2D1Factory1> d2d_factory_;
    Microsoft::WRL::ComPtr<ID2D1Device> d2d_device_;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2d_context_;

    // Placeholder for real camera texture
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> webcam_placeholder_brush_;

    Microsoft::WRL::ComPtr<IMFSourceReader> source_reader_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> webcam_texture_;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> webcam_bitmap_;
};

} // namespace cs

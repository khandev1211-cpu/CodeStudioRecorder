#include "click_animation_processor.h"
#include <algorithm>

namespace cs {

ClickAnimationProcessor::ClickAnimationProcessor() {}

void ClickAnimationProcessor::initializeD2D(ID3D11Device* device) {
    if (d2d_factory_) return;

    D2D1_FACTORY_OPTIONS options = {};
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, &d2d_factory_);

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);

    d2d_factory_->CreateDevice(dxgiDevice.Get(), &d2d_device_);
    d2d_device_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d_context_);

    d2d_context_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.6f), &ripple_brush_);
}

void ClickAnimationProcessor::addClick(float x, float y) {
    std::lock_guard<std::mutex> lock(clicks_mutex_);
    active_clicks_.push_back({ x, y, std::chrono::steady_clock::now(), 5.0f });
}

void ClickAnimationProcessor::process(VideoFrame& frame, ID3D11Device* device, ID3D11DeviceContext* context) {
    if (!enabled_ || !frame.data) return;

    std::lock_guard<std::mutex> lock(clicks_mutex_);
    if (active_clicks_.empty()) return;

    initializeD2D(device);

    ID3D11Texture2D* texture = static_cast<ID3D11Texture2D*>(frame.data);
    Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface;
    texture->QueryInterface(__uuidof(IDXGISurface), (void**)&dxgiSurface);

    Microsoft::WRL::ComPtr<ID2D1Bitmap1> targetBitmap;
    d2d_context_->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), nullptr, &targetBitmap);

    d2d_context_->SetTarget(targetBitmap.Get());
    d2d_context_->BeginDraw();

    auto now = std::chrono::steady_clock::now();

    for (auto it = active_clicks_.begin(); it != active_clicks_.end(); ) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->timestamp).count();

        if (elapsed > 500) { // Animation duration: 500ms
            it = active_clicks_.erase(it);
        } else {
            float progress = elapsed / 500.0f;
            float current_radius = 10.0f + (progress * 40.0f);
            float opacity = 0.6f * (1.0f - progress);

            ripple_brush_->SetOpacity(opacity);
            d2d_context_->DrawEllipse(
                D2D1::Ellipse(D2D1::Point2F(it->x, it->y), current_radius, current_radius),
                ripple_brush_.Get(),
                3.0f
            );
            ++it;
        }
    }

    d2d_context_->EndDraw();
    d2d_context_->SetTarget(nullptr);
}

} // namespace cs

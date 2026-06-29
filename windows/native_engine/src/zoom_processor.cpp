#include "zoom_processor.h"
#include <algorithm>
#include <windows.h>

namespace cs {

ZoomProcessor::ZoomProcessor() {}

void ZoomProcessor::initializeD2D(ID3D11Device* device) {
    if (d2d_factory_) return;

    D2D1_FACTORY_OPTIONS options = {};
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, &d2d_factory_);

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);

    d2d_factory_->CreateDevice(dxgiDevice.Get(), &d2d_device_);
    d2d_device_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d_context_);
}

void ZoomProcessor::process(VideoFrame& frame, ID3D11Device* device, ID3D11DeviceContext* context) {
    if (!enabled_ || !frame.data) return;

    initializeD2D(device);

    std::lock_guard<std::mutex> lock(mutex_);

    POINT p;
    GetCursorPos(&p);

    float target_x = (float)p.x;
    float target_y = (float)p.y;

    if (config_.target_hwnd != 0) {
        HWND hwnd = (HWND)config_.target_hwnd;
        if (IsWindow(hwnd)) {
            ScreenToClient(hwnd, &p);
            target_x = (float)p.x;
            target_y = (float)p.y;
        }
    }

    // Smoothing (Exponential Moving Average)
    // Adjust smoothing factor based on preference (0.0 to 1.0)
    const float lerp_factor = 0.15f;
    current_x_ = current_x_ + (target_x - current_x_) * lerp_factor;
    current_y_ = current_y_ + (target_y - current_y_) * lerp_factor;

    ID3D11Texture2D* source_texture = static_cast<ID3D11Texture2D*>(frame.data);

    // 1. Create temporary texture for composition if not exists or size changed
    D3D11_TEXTURE2D_DESC desc;
    source_texture->GetDesc(&desc);

    bool recreate_zoom_tex = false;
    if (!zoom_texture_) {
        recreate_zoom_tex = true;
    } else {
        D3D11_TEXTURE2D_DESC current_zoom_desc;
        zoom_texture_->GetDesc(&current_zoom_desc);
        if (current_zoom_desc.Width != desc.Width || current_zoom_desc.Height != desc.Height) {
            recreate_zoom_tex = true;
        }
    }

    if (recreate_zoom_tex) {
        zoom_texture_.Reset();
        device->CreateTexture2D(&desc, nullptr, &zoom_texture_);
    }

    // 2. Prepare Direct2D
    Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface;
    zoom_texture_->QueryInterface(__uuidof(IDXGISurface), (void**)&dxgiSurface);

    Microsoft::WRL::ComPtr<ID2D1Bitmap1> targetBitmap;
    d2d_context_->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), nullptr, &targetBitmap);

    Microsoft::WRL::ComPtr<IDXGISurface> srcSurface;
    source_texture->QueryInterface(__uuidof(IDXGISurface), (void**)&srcSurface);
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> sourceBitmap;
    d2d_context_->CreateBitmapFromDxgiSurface(srcSurface.Get(), nullptr, &sourceBitmap);

    d2d_context_->SetTarget(targetBitmap.Get());
    d2d_context_->BeginDraw();

    // Calculate source rect based on zoom level and smoothed cursor position
    float src_w = (float)desc.Width / zoom_level_;
    float src_h = (float)desc.Height / zoom_level_;

    float left = std::clamp(current_x_ - src_w / 2.0f, 0.0f, (float)desc.Width - src_w);
    float top = std::clamp(current_y_ - src_h / 2.0f, 0.0f, (float)desc.Height - src_h);

    D2D1_RECT_F srcRect = D2D1::RectF(left, top, left + src_w, top + src_h);
    D2D1_RECT_F destRect = D2D1::RectF(0, 0, (float)desc.Width, (float)desc.Height);

    d2d_context_->DrawBitmap(sourceBitmap.Get(), destRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, srcRect);

    d2d_context_->EndDraw();
    d2d_context_->SetTarget(nullptr);

    // 3. Copy zoomed content back to original frame
    context->CopyResource(source_texture, zoom_texture_.Get());
}

} // namespace cs

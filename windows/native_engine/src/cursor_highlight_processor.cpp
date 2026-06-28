#include "cursor_highlight_processor.h"
#include <windows.h>
#include <dxgi1_2.h>

namespace cs {

CursorHighlightProcessor::CursorHighlightProcessor() {}

void CursorHighlightProcessor::initializeD2D(ID3D11Device* device) {
    if (d2d_factory_) return;

    D2D1_FACTORY_OPTIONS options = {};
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, &d2d_factory_);

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);

    d2d_factory_->CreateDevice(dxgiDevice.Get(), &d2d_device_);
    d2d_device_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d_context_);

    d2d_context_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow, 0.5f), &highlight_brush_);
}

void CursorHighlightProcessor::process(VideoFrame& frame, ID3D11Device* device, ID3D11DeviceContext* context) {
    if (!enabled_) return;
    if (!frame.data) return;

    initializeD2D(device);

    ID3D11Texture2D* texture = static_cast<ID3D11Texture2D*>(frame.data);
    Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface;
    texture->QueryInterface(__uuidof(IDXGISurface), (void**)&dxgiSurface);

    Microsoft::WRL::ComPtr<ID2D1Bitmap1> targetBitmap;
    d2d_context_->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), nullptr, &targetBitmap);

    d2d_context_->SetTarget(targetBitmap.Get());
    d2d_context_->BeginDraw();

    POINT p;
    if (GetCursorPos(&p)) {
        // This assumes full screen capture for now.
        // For window capture, we would need to call ScreenToClient(hwnd, &p)
        // using the HWND of the captured window.

        d2d_context_->FillEllipse(
            D2D1::Ellipse(D2D1::Point2F((float)p.x, (float)p.y), 30.0f, 30.0f),
            highlight_brush_.Get()
        );
    }

    d2d_context_->EndDraw();
    d2d_context_->SetTarget(nullptr);
}

} // namespace cs

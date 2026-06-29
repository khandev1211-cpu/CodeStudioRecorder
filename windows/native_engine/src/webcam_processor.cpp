#include "webcam_processor.h"
#include <iostream>

namespace cs {

WebcamProcessor::WebcamProcessor() {}

WebcamProcessor::~WebcamProcessor() {
    stopCamera();
}

void WebcamProcessor::setEnabled(bool enabled) {
    if (enabled == enabled_) return;
    enabled_ = enabled;
    if (enabled_) startCamera();
    else stopCamera();
}

void WebcamProcessor::startCamera() {
    // TODO: Initialize Media Foundation and start capture
}

void WebcamProcessor::stopCamera() {
    // TODO: Release camera resources
}

void WebcamProcessor::initializeD2D(ID3D11Device* device) {
    if (d2d_factory_) return;

    D2D1_FACTORY_OPTIONS options = {};
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, &d2d_factory_);

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);

    d2d_factory_->CreateDevice(dxgiDevice.Get(), &d2d_device_);
    d2d_device_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d_context_);

    d2d_context_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 0.8f), &webcam_placeholder_brush_);
}

void WebcamProcessor::process(VideoFrame& frame, ID3D11Device* device, ID3D11DeviceContext* context) {
    if (!enabled_ || !frame.data) return;

    initializeD2D(device);

    ID3D11Texture2D* texture = static_cast<ID3D11Texture2D*>(frame.data);
    Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface;
    texture->QueryInterface(__uuidof(IDXGISurface), (void**)&dxgiSurface);

    Microsoft::WRL::ComPtr<ID2D1Bitmap1> targetBitmap;
    d2d_context_->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), nullptr, &targetBitmap);

    d2d_context_->SetTarget(targetBitmap.Get());
    d2d_context_->BeginDraw();

    std::lock_guard<std::mutex> lock(mutex_);

    // Draw stylized placeholder for webcam
    d2d_context_->FillRectangle(rect_, webcam_placeholder_brush_.Get());

    // Draw "Webcam" text placeholder
    // In a real implementation, we would draw the frame from the camera here

    d2d_context_->EndDraw();
    d2d_context_->SetTarget(nullptr);
}

} // namespace cs

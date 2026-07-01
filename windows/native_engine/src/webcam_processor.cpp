#include "webcam_processor.h"
#include <iostream>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <codecvt>
#include <locale>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mfreadwrite.lib")

namespace cs {

WebcamProcessor::WebcamProcessor() {
    MFStartup(MF_VERSION);
}

WebcamProcessor::~WebcamProcessor() {
    stopCamera();
    MFShutdown();
}

void WebcamProcessor::setEnabled(bool enabled) {
    if (enabled == enabled_) return;
    enabled_ = enabled;
    if (enabled_) startCamera();
    else stopCamera();
}

void WebcamProcessor::startCamera() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (source_reader_) return;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFActivate> device;

    // 1. Find the device
    IMFAttributes* attributes = nullptr;
    MFCreateAttributes(&attributes, 1);
    attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);

    IMFActivate** devices = nullptr;
    UINT32 count = 0;
    MFEnumDeviceSources(attributes, &devices, &count);

    if (config_.webcam_device_id && strlen(config_.webcam_device_id) > 0) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::wstring targetId = converter.from_bytes(config_.webcam_device_id);

        for (UINT32 i = 0; i < count; i++) {
            LPWSTR id = nullptr;
            devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &id, nullptr);
            if (id && targetId == id) {
                device = devices[i];
            }
            if (id) CoTaskMemFree(id);
        }
    } else if (count > 0) {
        device = devices[0];
    }

    // Cleanup other devices
    for (UINT32 i = 0; i < count; i++) {
        if (devices[i] != device.Get()) devices[i]->Release();
    }
    CoTaskMemFree(devices);
    attributes->Release();

    if (!device) return;

    // 2. Create source reader
    Microsoft::WRL::ComPtr<IMFMediaSource> source;
    device->ActivateObject(__uuidof(IMFMediaSource), (void**)&source);

    Microsoft::WRL::ComPtr<IMFAttributes> readerAttrs;
    MFCreateAttributes(&readerAttrs, 3);
    readerAttrs->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
    readerAttrs->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
    readerAttrs->SetUINT32(MF_SOURCE_READER_DISCONNECT_MEDIASOURCE_ON_SHUTDOWN, TRUE);

    MFCreateSourceReaderFromMediaSource(source.Get(), readerAttrs.Get(), source_reader_.GetAddressOf());

    // 3. Set output format to RGB32
    Microsoft::WRL::ComPtr<IMFMediaType> type;
    MFCreateMediaType(type.GetAddressOf());
    type->SetGUID(MF_MT_MAJOR_TYPE, MF_MT_VIDEO);
    type->SetGUID(MF_MT_SUBTYPE, MF_MT_RGB32);
    source_reader_->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, type.Get());
}

void WebcamProcessor::stopCamera() {
    std::lock_guard<std::mutex> lock(mutex_);
    source_reader_.Reset();
    webcam_texture_.Reset();
    webcam_bitmap_.Reset();
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

    std::lock_guard<std::mutex> lock(mutex_);
    if (!source_reader_) return;

    // 1. Try to get a new frame from webcam
    DWORD streamIndex, flags;
    LONGLONG timestamp;
    Microsoft::WRL::ComPtr<IMFSample> sample;

    HRESULT hr = source_reader_->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &flags, &timestamp, sample.GetAddressOf());

    if (SUCCEEDED(hr) && sample) {
        Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;
        sample->ConvertToContiguousBuffer(buffer.GetAddressOf());

        BYTE* data = nullptr;
        DWORD maxLength = 0, currentLength = 0;
        buffer->Lock(&data, &maxLength, &currentLength);

        // Get format info for dimensions
        Microsoft::WRL::ComPtr<IMFMediaType> type;
        source_reader_->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, type.GetAddressOf());
        UINT32 w, h;
        MFGetAttributeSize(type.Get(), MF_MT_FRAME_SIZE, &w, &h);

        // Update texture if size changed or first frame
        if (!webcam_texture_) {
            D3D11_TEXTURE2D_DESC desc = {};
            desc.Width = w;
            desc.Height = h;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            device->CreateTexture2D(&desc, nullptr, &webcam_texture_);

            Microsoft::WRL::ComPtr<IDXGISurface> dxgiWebcamSurface;
            webcam_texture_->QueryInterface(__uuidof(IDXGISurface), (void**)&dxgiWebcamSurface);
            d2d_context_->CreateBitmapFromDxgiSurface(dxgiWebcamSurface.Get(), nullptr, &webcam_bitmap_);
        }

        if (webcam_texture_) {
            D3D11_MAPPED_SUBRESOURCE mapped;
            if (SUCCEEDED(context->Map(webcam_texture_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
                // Copy data (simplified, assuming pitch matches)
                for (UINT32 y = 0; y < h; y++) {
                    memcpy((BYTE*)mapped.pData + y * mapped.RowPitch, data + y * w * 4, w * 4);
                }
                context->Unmap(webcam_texture_.Get(), 0);
            }
        }

        buffer->Unlock();
    }

    // 2. Draw the webcam bitmap on top of the main frame
    ID3D11Texture2D* texture = static_cast<ID3D11Texture2D*>(frame.data);
    Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface;
    texture->QueryInterface(__uuidof(IDXGISurface), (void**)&dxgiSurface);

    Microsoft::WRL::ComPtr<ID2D1Bitmap1> targetBitmap;
    d2d_context_->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), nullptr, &targetBitmap);

    d2d_context_->SetTarget(targetBitmap.Get());
    d2d_context_->BeginDraw();

    if (webcam_bitmap_) {
        d2d_context_->DrawBitmap(webcam_bitmap_.Get(), rect_);
    } else {
        d2d_context_->FillRectangle(rect_, webcam_placeholder_brush_.Get());
    }

    d2d_context_->EndDraw();
    d2d_context_->SetTarget(nullptr);
}

} // namespace cs

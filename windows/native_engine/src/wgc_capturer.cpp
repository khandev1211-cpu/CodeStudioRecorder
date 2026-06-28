#include "wgc_capturer.h"
#include <inspectable.h>
#include <unknwn.h>
#include <winrt/base.h>

namespace cs {

WGCCapturer::WGCCapturer() {
    winrt::init_apartment();
}

WGCCapturer::~WGCCapturer() {
    stop();
}

bool WGCCapturer::initialize(const RecordingConfig& config) {
    // 1. Initialize D3D11 Device
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION, &d3d_device_, nullptr, nullptr);

    if (FAILED(hr)) return false;

    // 2. Wrap D3D11 Device for WinRT
    auto dxgiDevice = d3d_device_.As<IDXGIDevice>();
    winrt::com_ptr<::IInspectable> inspectable;
    hr = CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.Get(), inspectable.put());
    if (FAILED(hr)) return false;

    device_ = inspectable.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

    return true;
}

void WGCCapturer::start(FrameCallback callback) {
    callback_ = callback;

    // For now, this is a skeleton.
    // In a real implementation, we would use GraphicsCapturePicker or CaptureItem from a window handle.
    // For the sake of this implementation, we assume we want to capture the primary monitor.
}

void WGCCapturer::stop() {
    if (session_) {
        session_.Close();
        session_ = nullptr;
    }
    if (frame_pool_) {
        frame_pool_.Close();
        frame_pool_ = nullptr;
    }
}

void WGCCapturer::pause() {}
void WGCCapturer::resume() {}

void WGCCapturer::onFrameArrived(
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
    winrt::Windows::Foundation::IInspectable const& args)
{
    auto frame = sender.TryGetNextFrame();
    if (!frame) return;

    VideoFrame vf;
    vf.width = frame.ContentSize().Width;
    vf.height = frame.ContentSize().Height;
    vf.timestamp_qpc = 0; // Should get from frame.SystemRelativeTime()
    vf.data = nullptr; // Texture access

    if (callback_) {
        callback_(vf);
    }
}

} // namespace cs

#include "wgc_capturer.h"
#include <inspectable.h>
#include <unknwn.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3d11.h>
#include <dwmapi.h>

namespace cs {

WGCCapturer::WGCCapturer() {
    winrt::init_apartment();
}

WGCCapturer::~WGCCapturer() {
    stop();
}

bool WGCCapturer::initialize(const RecordingConfig& config) {
    // 1. Initialize D3D11 Device
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
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

    // For the MVP, we'll try to capture the foreground window or primary monitor.
    // In this snippet, we'll use a hack to get the primary monitor if no item is set.

    // Note: To capture a specific window, we'd use:
    // HWND hwnd = GetForegroundWindow();
    // auto interop = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
    // hr = interop->CreateForWindow(hwnd, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), reinterpret_cast<void**>(winrt::put_abi(item_)));

    // For now, let's assume 'item_' was somehow selected or we are capturing a default.
    if (!item_) {
        // Mocking item selection for now - in reality, we need user selection or a specific monitor handle
        return;
    }

    frame_pool_ = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(
        device_,
        winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
        2,
        item_.Size());

    session_ = frame_pool_.CreateCaptureSession(item_);

    frame_arrived_revoker_ = frame_pool_.FrameArrived(winrt::auto_revoke, { this, &WGCCapturer::onFrameArrived });

    session_.StartCapture();
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

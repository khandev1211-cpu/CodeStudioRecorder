#include "wgc_capturer.h"
#include <inspectable.h>
#include <unknwn.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3d11.h>
#include <dwmapi.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.capture.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <dxgi.h>

// Manually define the interop interface if it's not found in the headers
#ifndef __IDirect3DDXGIInterfaceAccess_INTERFACE_DEFINED__
struct __declspec(uuid("A9B3D012-3DF2-4EE3-B8D1-8695F457D3C1"))
IDirect3DDXGIInterfaceAccess : ::IUnknown
{
    virtual HRESULT __stdcall GetInterface(GUID const& id, void** object) = 0;
};
#endif

namespace cs {

WGCCapturer::WGCCapturer() {
    try {
        winrt::init_apartment(winrt::apartment_type::multi_threaded);
    } catch (...) {
        // Already initialized, ignore
    }
}

WGCCapturer::~WGCCapturer() {
    stop();
}

bool WGCCapturer::initialize(const RecordingConfig& config) {
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION, &d3d_device_, nullptr, nullptr);

    if (FAILED(hr)) return false;

    d3d_device_->GetImmediateContext(&d3d_context_);

    // Use winrt::com_ptr for COM interfaces
    winrt::com_ptr<IDXGIDevice> dxgiDevice;
    hr = d3d_device_->QueryInterface(__uuidof(IDXGIDevice), dxgiDevice.put_void());
    if (FAILED(hr)) return false;

    winrt::com_ptr<::IInspectable> inspectable;
    hr = CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), inspectable.put());
    if (FAILED(hr)) return false;

    device_ = inspectable.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

    if (config.target_hwnd != 0) {
        auto interop = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
        hr = interop->CreateForWindow(reinterpret_cast<HWND>(config.target_hwnd), winrt::guid_of<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>(), reinterpret_cast<void**>(winrt::put_abi(item_)));
        if (FAILED(hr)) return false;
    } else {
        // Full screen (Primary Monitor)
        HMONITOR hMonitor = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);
        auto interop = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
        hr = interop->CreateForMonitor(hMonitor, winrt::guid_of<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>(), reinterpret_cast<void**>(winrt::put_abi(item_)));
        if (FAILED(hr)) return false;
    }

    return true;
}

void WGCCapturer::start(FrameCallback callback) {
    callback_ = callback;
    if (!item_) return;

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
    if (session_) { session_.Close(); session_ = nullptr; }
    if (frame_pool_) { frame_pool_.Close(); frame_pool_ = nullptr; }
}

void WGCCapturer::pause() {}
void WGCCapturer::resume() {}

void WGCCapturer::onFrameArrived(
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
    winrt::Windows::Foundation::IInspectable const& args)
{
    auto frame = sender.TryGetNextFrame();
    if (!frame) return;

    auto surface = frame.Surface();

    // Get the raw IUnknown from the WinRT surface
    auto unk = surface.as<::IUnknown>();

    // Query for the interop interface to get the underlying DXGI texture
    winrt::com_ptr<IDirect3DDXGIInterfaceAccess> access;
    HRESULT hr = unk->QueryInterface(__uuidof(IDirect3DDXGIInterfaceAccess), access.put_void());

    if (SUCCEEDED(hr)) {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
        hr = access->GetInterface(__uuidof(ID3D11Texture2D), (void**)&texture);

        if (SUCCEEDED(hr)) {
            VideoFrame vf;
            vf.width = frame.ContentSize().Width;
            vf.height = frame.ContentSize().Height;
            vf.timestamp_qpc = frame.SystemRelativeTime().count();
            vf.data = texture.Get();

            if (callback_) {
                callback_(vf);
            }
        }
    }
}

} // namespace cs

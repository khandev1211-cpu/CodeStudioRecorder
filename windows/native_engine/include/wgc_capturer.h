#pragma once
#include "i_capturer.h"
#include <windows.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3d11.h>
#include <d3d11.h>
#include <wrl/client.h>

namespace cs {

class WGCCapturer : public ICapturer {
public:
    WGCCapturer();
    ~WGCCapturer();

    bool initialize(const RecordingConfig& config) override;
    void start(FrameCallback callback) override;
    void stop() override;
    void pause() override;
    void resume() override;

private:
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem item_{ nullptr };
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession session_{ nullptr };
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool frame_pool_{ nullptr };

    Microsoft::WRL::ComPtr<ID3D11Device> d3d_device_;
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice device_{ nullptr };

    void onFrameArrived(winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender, winrt::Windows::Foundation::IInspectable const& args);

    FrameCallback callback_;
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::FrameArrived_revoker frame_arrived_revoker_;
};

} // namespace cs

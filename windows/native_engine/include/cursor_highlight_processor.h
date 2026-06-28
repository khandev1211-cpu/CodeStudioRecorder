#pragma once
#include "i_frame_processor.h"
#include <atomic>
#include <wrl/client.h>
#include <d2d1_1.h>

namespace cs {

class CursorHighlightProcessor : public IFrameProcessor {
public:
    CursorHighlightProcessor();
    ~CursorHighlightProcessor() = default;

    void process(VideoFrame& frame, ID3D11Device* device, ID3D11DeviceContext* context) override;

    bool isEnabled() const override { return enabled_; }
    void setEnabled(bool enabled) override { enabled_ = enabled; }

private:
    void initializeD2D(ID3D11Device* device);

    std::atomic<bool> enabled_{ true };

    Microsoft::WRL::ComPtr<ID2D1Factory1> d2d_factory_;
    Microsoft::WRL::ComPtr<ID2D1Device> d2d_device_;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2d_context_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> highlight_brush_;
};

} // namespace cs

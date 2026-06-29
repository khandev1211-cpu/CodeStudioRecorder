#pragma once
#include "i_frame_processor.h"
#include <vector>
#include <mutex>
#include <wrl/client.h>
#include <d2d1_1.h>
#include <chrono>

namespace cs {

struct ClickEvent {
    float x, y;
    std::chrono::steady_clock::time_point timestamp;
    float radius;
};

class ClickAnimationProcessor : public IFrameProcessor {
public:
    ClickAnimationProcessor();
    ~ClickAnimationProcessor() = default;

    void process(VideoFrame& frame, ID3D11Device* device, ID3D11DeviceContext* context) override;

    bool isEnabled() const override { return enabled_; }
    void setEnabled(bool enabled) override { enabled_ = enabled; }

    void addClick(float x, float y);

private:
    void initializeD2D(ID3D11Device* device);

    bool enabled_ = true;
    std::vector<ClickEvent> active_clicks_;
    std::mutex clicks_mutex_;

    Microsoft::WRL::ComPtr<ID2D1Factory1> d2d_factory_;
    Microsoft::WRL::ComPtr<ID2D1Device> d2d_device_;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2d_context_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> ripple_brush_;
};

} // namespace cs

#pragma once
#include "i_frame_processor.h"
#include <vector>
#include <mutex>
#include <string>
#include <wrl/client.h>
#include <d2d1_1.h>

namespace cs {

enum class AnnotationType {
    Line,
    Rectangle,
    Ellipse,
    Arrow
};

struct AnnotationShape {
    AnnotationType type;
    float x1, y1, x2, y2;
    uint32_t color; // 0xAARRGGBB
    float stroke_width;
};

class AnnotationProcessor : public IFrameProcessor {
public:
    AnnotationProcessor();
    ~AnnotationProcessor() = default;

    void onStart(const RecordingConfig& config) override {
        config_ = config;
    }

    void process(VideoFrame& frame, ID3D11Device* device, ID3D11DeviceContext* context) override;

    bool isEnabled() const override { return enabled_; }
    void setEnabled(bool enabled) override { enabled_ = enabled; }

    void addShape(const AnnotationShape& shape);
    void clear();
    void undo();
    std::vector<AnnotationShape> getShapes();

private:
    void initializeD2D(ID3D11Device* device);
    void drawArrow(const AnnotationShape& shape);

    bool enabled_ = true;
    RecordingConfig config_{};
    std::vector<AnnotationShape> shapes_;
    std::mutex mutex_;

    Microsoft::WRL::ComPtr<ID2D1Factory1> d2d_factory_;
    Microsoft::WRL::ComPtr<ID2D1Device> d2d_device_;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2d_context_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush_;
};

} // namespace cs

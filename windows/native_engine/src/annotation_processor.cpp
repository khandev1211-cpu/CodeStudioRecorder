#include "annotation_processor.h"
#include <cmath>

namespace cs {

AnnotationProcessor::AnnotationProcessor() {}

void AnnotationProcessor::initializeD2D(ID3D11Device* device) {
    if (d2d_factory_) return;

    D2D1_FACTORY_OPTIONS options = {};
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, &d2d_factory_);

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);

    d2d_factory_->CreateDevice(dxgiDevice.Get(), &d2d_device_);
    d2d_device_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d_context_);

    d2d_context_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &brush_);
}

void AnnotationProcessor::addShape(const AnnotationShape& shape) {
    std::lock_guard<std::mutex> lock(mutex_);
    shapes_.push_back(shape);
}

void AnnotationProcessor::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    shapes_.clear();
}

void AnnotationProcessor::undo() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!shapes_.empty()) shapes_.pop_back();
}

void AnnotationProcessor::process(VideoFrame& frame, ID3D11Device* device, ID3D11DeviceContext* context) {
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
    for (const auto& shape : shapes_) {
        // Extract color: 0xAARRGGBB
        float a = ((shape.color >> 24) & 0xFF) / 255.0f;
        float r = ((shape.color >> 16) & 0xFF) / 255.0f;
        float g = ((shape.color >> 8) & 0xFF) / 255.0f;
        float b = (shape.color & 0xFF) / 255.0f;
        brush_->SetColor(D2D1::ColorF(r, g, b, a));

        switch (shape.type) {
            case AnnotationType::Line:
                d2d_context_->DrawLine(
                    D2D1::Point2F(shape.x1, shape.y1),
                    D2D1::Point2F(shape.x2, shape.y2),
                    brush_.Get(),
                    shape.stroke_width
                );
                break;
            case AnnotationType::Rectangle:
                d2d_context_->DrawRectangle(
                    D2D1::RectF(shape.x1, shape.y1, shape.x2, shape.y2),
                    brush_.Get(),
                    shape.stroke_width
                );
                break;
            case AnnotationType::Ellipse:
                d2d_context_->DrawEllipse(
                    D2D1::Ellipse(
                        D2D1::Point2F((shape.x1 + shape.x2) / 2, (shape.y1 + shape.y2) / 2),
                        std::abs(shape.x2 - shape.x1) / 2,
                        std::abs(shape.y2 - shape.y1) / 2
                    ),
                    brush_.Get(),
                    shape.stroke_width
                );
                break;
            case AnnotationType::Arrow:
                drawArrow(shape);
                break;
        }
    }

    d2d_context_->EndDraw();
    d2d_context_->Flush();
    d2d_context_->SetTarget(nullptr);
}

void AnnotationProcessor::drawArrow(const AnnotationShape& shape) {
    // Draw main shaft
    d2d_context_->DrawLine(
        D2D1::Point2F(shape.x1, shape.y1),
        D2D1::Point2F(shape.x2, shape.y2),
        brush_.Get(),
        shape.stroke_width
    );

    // Draw arrowhead
    float angle = std::atan2(shape.y2 - shape.y1, shape.x2 - shape.x1);
    float head_len = 15.0f + shape.stroke_width * 2;

    d2d_context_->DrawLine(
        D2D1::Point2F(shape.x2, shape.y2),
        D2D1::Point2F(shape.x2 - head_len * std::cos(angle - 0.5f), shape.y2 - head_len * std::sin(angle - 0.5f)),
        brush_.Get(),
        shape.stroke_width
    );
    d2d_context_->DrawLine(
        D2D1::Point2F(shape.x2, shape.y2),
        D2D1::Point2F(shape.x2 - head_len * std::cos(angle + 0.5f), shape.y2 - head_len * std::sin(angle + 0.5f)),
        brush_.Get(),
        shape.stroke_width
    );
}

} // namespace cs

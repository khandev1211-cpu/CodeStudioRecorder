#pragma once
#include <windows.h>
#include <d2d1_1.h>
#include <wrl/client.h>
#include <vector>
#include <mutex>
#include <atomic>
#include "annotation_processor.h"

namespace cs {

class OverlayManager {
public:
    static OverlayManager& instance();

    void start();
    void stop();

    void updateAnnotations(const std::vector<AnnotationShape>& shapes);
    void updateCursorPosition(float x, float y);
    void setHighlightEnabled(bool enabled) { highlight_enabled_ = enabled; }

private:
    OverlayManager() = default;
    ~OverlayManager();

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void renderLoop();
    void render();

    HWND hwnd_ = nullptr;
    std::thread render_thread_;
    std::atomic<bool> running_{ false };

    Microsoft::WRL::ComPtr<ID2D1Factory1> d2d_factory_;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> render_target_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> highlight_brush_;

    std::vector<AnnotationShape> current_shapes_;
    float cursor_x_ = 0, cursor_y_ = 0;
    bool highlight_enabled_ = false;
    std::mutex mutex_;
};

} // namespace cs

#include "overlay_manager.h"
#include "cs_logger.h"
#include <thread>

namespace cs {

OverlayManager& OverlayManager::instance() {
    static OverlayManager inst;
    return inst;
}

OverlayManager::~OverlayManager() {
    stop();
}

void OverlayManager::start() {
    if (running_) return;
    running_ = true;
    render_thread_ = std::thread(&OverlayManager::renderLoop, this);
}

void OverlayManager::stop() {
    running_ = false;
    if (render_thread_.joinable()) {
        render_thread_.join();
    }
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

void OverlayManager::updateAnnotations(const std::vector<AnnotationShape>& shapes) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_shapes_ = shapes;
}

void OverlayManager::updateCursorPosition(float x, float y) {
    std::lock_guard<std::mutex> lock(mutex_);
    cursor_x_ = x;
    cursor_y_ = y;
}

void OverlayManager::renderLoop() {
    // 1. Register and Create Window
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"CodeStudioOverlay";
    RegisterClassExW(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    hwnd_ = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE,
        wc.lpszClassName, L"",
        WS_POPUP,
        0, 0, screenWidth, screenHeight,
        nullptr, nullptr, wc.hInstance, nullptr
    );

    // Set transparency
    SetLayeredWindowAttributes(hwnd_, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(hwnd_, SW_SHOW);

    // 2. Initialize D2D
    D2D1_FACTORY_OPTIONS options = {};
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &options, reinterpret_cast<void**>(d2d_factory_.GetAddressOf()));

    // Create a render target for the window
    RECT rc;
    GetClientRect(hwnd_, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    d2d_factory_->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)),
        D2D1::HwndRenderTargetProperties(hwnd_, size),
        &render_target_
    );

    render_target_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &brush_);
    render_target_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow, 0.4f), &highlight_brush_);

    // 3. Main Loop
    MSG msg = {};
    while (running_) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60fps
    }
}

void OverlayManager::render() {
    if (!render_target_) return;

    render_target_->BeginDraw();
    render_target_->Clear(D2D1::ColorF(0, 0, 0, 0));

    std::lock_guard<std::mutex> lock(mutex_);

    // Draw Cursor Highlight if enabled
    if (highlight_enabled_) {
        render_target_->FillEllipse(
            D2D1::Ellipse(D2D1::Point2F(cursor_x_, cursor_y_), 30.0f, 30.0f),
            highlight_brush_.Get()
        );
    }

    // Draw Annotations
    for (const auto& shape : current_shapes_) {
        float a = ((shape.color >> 24) & 0xFF) / 255.0f;
        float r = ((shape.color >> 16) & 0xFF) / 255.0f;
        float g = ((shape.color >> 8) & 0xFF) / 255.0f;
        float b = (shape.color & 0xFF) / 255.0f;
        brush_->SetColor(D2D1::ColorF(r, g, b, a));

        switch (shape.type) {
            case AnnotationType::Line:
                render_target_->DrawLine(D2D1::Point2F(shape.x1, shape.y1), D2D1::Point2F(shape.x2, shape.y2), brush_.Get(), shape.stroke_width);
                break;
            case AnnotationType::Rectangle:
                render_target_->DrawRectangle(D2D1::RectF(shape.x1, shape.y1, shape.x2, shape.y2), brush_.Get(), shape.stroke_width);
                break;
            case AnnotationType::Ellipse:
                render_target_->DrawEllipse(D2D1::Ellipse(D2D1::Point2F((shape.x1 + shape.x2) / 2, (shape.y1 + shape.y2) / 2), std::abs(shape.x2 - shape.x1) / 2, std::abs(shape.y2 - shape.y1) / 2), brush_.Get(), shape.stroke_width);
                break;
            case AnnotationType::Arrow:
                // Simple arrow for overlay
                render_target_->DrawLine(D2D1::Point2F(shape.x1, shape.y1), D2D1::Point2F(shape.x2, shape.y2), brush_.Get(), shape.stroke_width);
                break;
        }
    }

    render_target_->EndDraw();
}

LRESULT CALLBACK OverlayManager::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

} // namespace cs

#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <functional>
#include <mutex>

namespace cs {

using MouseClickCallback = std::function<void(float x, float y)>;

class GlobalMouseHook {
public:
    static GlobalMouseHook& instance();

    void start(MouseClickCallback callback);
    void stop();

private:
    GlobalMouseHook() = default;
    ~GlobalMouseHook();

    static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);

    HHOOK hook_ = nullptr;
    MouseClickCallback callback_;
    std::mutex mutex_;
};

} // namespace cs

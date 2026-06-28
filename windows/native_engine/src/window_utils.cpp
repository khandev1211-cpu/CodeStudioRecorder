// 1. Standard headers first
#include <vector>
#include <string>

// 2. Project headers
#include "window_utils.h"

// 3. System headers last
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace cs {

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    auto* windows = reinterpret_cast<std::vector<WindowInfo>*>(lParam);

    if (!IsWindowVisible(hwnd) || GetWindowTextLength(hwnd) == 0) {
        return TRUE;
    }

    wchar_t title[256];
    GetWindowTextW(hwnd, title, 256);

    wchar_t className[256];
    GetClassNameW(hwnd, className, 256);

    windows->push_back({ hwnd, title, className });
    return TRUE;
}

std::vector<WindowInfo> WindowUtils::enumerateWindows() {
    std::vector<WindowInfo> windows;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));
    return windows;
}

} // namespace cs

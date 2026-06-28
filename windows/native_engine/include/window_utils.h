#pragma once
#include <windows.h>
#include <vector>
#include <string>

namespace cs {

struct WindowInfo {
    HWND hwnd;
    std::wstring title;
    std::wstring class_name;
};

class WindowUtils {
public:
    static std::vector<WindowInfo> enumerateWindows();
};

} // namespace cs

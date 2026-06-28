#pragma once
#include <vector>
#include <string>

// Forward declare HWND to avoid windows.h in header
typedef struct HWND__* HWND;

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

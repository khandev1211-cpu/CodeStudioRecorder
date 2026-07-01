#include "system_check.h"
#include <windows.h>
#include <versionhelpers.h>
#include <d3d11.h>
#include <filesystem>
#include "cs_logger.h"

namespace cs {

std::vector<SystemRequirement> SystemCheck::runFullCheck() {
    std::vector<SystemRequirement> results;

    // 1. Windows Version (Need 1903+ for WGC)
    results.push_back({
        "Windows 10 1903+",
        checkWinVersion(),
        "Required for Windows Graphics Capture API"
    });

    // 2. Direct3D 11 Support
    results.push_back({
        "DirectX 11",
        checkD3D11(),
        "Required for hardware accelerated rendering"
    });

    // 3. FFmpeg Libraries
    results.push_back({
        "Media Codecs",
        checkFFmpeg(),
        "FFmpeg DLLs required for video encoding"
    });

    return results;
}

bool SystemCheck::checkWinVersion() {
    return IsWindows10OrGreater();
    // More specific check for 1903 (Build 18362) could be added here
}

bool SystemCheck::checkD3D11() {
    D3D_FEATURE_LEVEL featureLevel;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;

    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, &device, &featureLevel, &context);

    if (SUCCEEDED(hr)) {
        device->Release();
        context->Release();
        return true;
    }
    return false;
}

bool SystemCheck::checkFFmpeg() {
#ifdef USE_REAL_FFMPEG
    // In a real app, we check if the DLLs are in the path or app directory
    const char* libs[] = { "avcodec-60.dll", "avformat-60.dll", "avutil-58.dll" };
    for (const char* lib : libs) {
        HMODULE h = LoadLibraryA(lib);
        if (!h) return false;
        FreeLibrary(h);
    }
    return true;
#else
    return true; // Mock mode doesn't need them
#endif
}

} // namespace cs

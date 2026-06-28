#include "encoder_factory.h"
#include "ffmpeg_encoder.h"
#include "cs_logger.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace cs {

bool probeNVENC() {
#ifdef _WIN32
    HMODULE hLib = LoadLibraryA("nvEncodeAPI64.dll");
    if (hLib) {
        FreeLibrary(hLib);
        return true;
    }
#endif
    return false;
}

bool probeQuickSync() {
#ifdef _WIN32
    HMODULE hLib = LoadLibraryA("libmfxsw64.dll"); // Or hardware variant
    if (hLib) {
        FreeLibrary(hLib);
        return true;
    }
#endif
    return false;
}

std::unique_ptr<IEncoder> EncoderFactory::createEncoder(EncoderType type) {
    if (type == EncoderType::Auto) {
        if (probeNVENC()) {
            CS_LOG_INFO("Hardware Encoder Detected: NVIDIA NVENC");
            // In a real build, we\u0027d return a specialized NVENCEncoder
        } else if (probeQuickSync()) {
            CS_LOG_INFO("Hardware Encoder Detected: Intel QuickSync");
        } else {
            CS_LOG_INFO("No hardware encoder detected, falling back to software");
        }
    }

    return std::make_unique<FFmpegEncoder>();
}

} // namespace cs

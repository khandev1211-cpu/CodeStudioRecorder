#pragma once
#include <cstdint>

#ifdef _WIN32
#define CSE_API extern "C" __declspec(dllexport)
#else
#define CSE_API extern "C" __attribute__((visibility("default")))
#endif

// Plain C struct for FFI - outside namespace for absolute safety with FFI
struct NativeWindowInfo {
    int64_t hwnd;
    const char* title;
};

// Forward declaration of cs::RecordingConfig to avoid including heavy headers here
namespace cs { struct RecordingConfig; struct RecordingStats; }

CSE_API int32_t cse_start_recording(cs::RecordingConfig* config);
CSE_API int32_t cse_stop_recording();
CSE_API int32_t cse_pause_recording();
CSE_API int32_t cse_resume_recording();
CSE_API void cse_get_stats(cs::RecordingStats* stats);
CSE_API int32_t cse_get_status();

typedef void (*WindowCallback)(NativeWindowInfo*);
CSE_API void cse_enumerate_windows(WindowCallback callback);

CSE_API void cse_set_setting_string(const char* key, const char* value);
CSE_API const char* cse_get_setting_string(const char* key, const char* default_value);
CSE_API void cse_set_setting_int(const char* key, int32_t value);
CSE_API int32_t cse_get_setting_int(const char* key, int32_t default_value);

CSE_API void cse_set_processor_enabled(int32_t index, bool enabled);

#pragma once
#include "cs_types.h"

#ifdef _WIN32
#define CSE_API extern "C" __declspec(dllexport)
#else
#define CSE_API extern "C" __attribute__((visibility("default")))
#endif

CSE_API int32_t cse_start_recording(cs::RecordingConfig* config);
CSE_API int32_t cse_stop_recording();
CSE_API int32_t cse_pause_recording();
CSE_API int32_t cse_resume_recording();
CSE_API void cse_get_stats(cs::RecordingStats* stats);
CSE_API int32_t cse_get_status();

# 13 — API Reference

## CodeStudio Recorder — Internal API Reference

---

## Overview

This document covers all internal APIs, interfaces, modules, and FFI-exported contracts that form the public surface area of the CodeStudio Recorder engine.

---

## FFI Exports (C ABI)

All functions exported from `codestudio_engine.dll` use `extern "C"` linkage and `__cdecl` calling convention.

### Engine Lifecycle

```c
// Initialize engine (must be called first)
int32_t cse_initialize(const CSEngineConfig* config);

// Shutdown engine and release all resources
void cse_shutdown();

// Get engine version string
const char* cse_version();
```

### Recording Control

```c
// Begin a recording session with the provided configuration
// Returns 0 on success, error code on failure
int32_t cse_start_recording(const CSRecordingConfig* config);

// Pause the current recording
int32_t cse_pause_recording();

// Resume a paused recording
int32_t cse_resume_recording();

// Stop recording, flush, and finalize the output file
// Blocks until file is complete (or timeout ms)
int32_t cse_stop_recording(uint32_t timeoutMs);

// Get current recording state
CSRecordingState cse_get_state();

// Get live recording stats (non-blocking, call from polling loop)
void cse_get_stats(CSRecordingStats* out);
```

### Configuration

```c
// Load settings from database
int32_t cse_load_settings();

// Save a setting value (key-value, UTF-8)
int32_t cse_set_setting(const char* key, const char* value);

// Get a setting value (caller must NOT free returned pointer)
const char* cse_get_setting(const char* key, const char* defaultValue);
```

### Callbacks (Dart registers these via FFI)

```c
typedef void (*CSStateChangedCallback)(CSRecordingState newState, void* userdata);
typedef void (*CSErrorCallback)(int32_t errorCode, const char* message, void* userdata);
typedef void (*CSProgressCallback)(double progress, void* userdata);  // for exports

int32_t cse_register_state_callback(CSStateChangedCallback cb, void* userdata);
int32_t cse_register_error_callback(CSErrorCallback cb, void* userdata);
int32_t cse_register_progress_callback(CSProgressCallback cb, void* userdata);
```

---

## C Structs (FFI Boundary)

```c
typedef struct {
    uint32_t fps;
    uint32_t width;
    uint32_t height;
    int32_t  codec;           // CSVideoCodec enum
    int32_t  rateControlMode; // CSRateControlMode enum
    uint32_t bitrate;
    uint32_t maxBitrate;
    int32_t  captureMode;     // CSCaptureMode enum
    uint64_t captureHandle;   // HWND for window capture
    bool     micEnabled;
    bool     systemAudioEnabled;
    float    micGain;
    float    systemGain;
    const char* outputPath;   // UTF-8
} CSRecordingConfig;

typedef struct {
    int64_t  elapsedMs;
    uint64_t framesCaptured;
    uint64_t framesEncoded;
    uint64_t framesDropped;
    uint32_t currentFps;
    float    encoderLoadPercent;
    uint64_t fileSizeBytes;
    float    audioPeakDb;
} CSRecordingStats;
```

---

## C++ Internal Interfaces

### IVideoEncoder

```cpp
class IVideoEncoder {
public:
    virtual ~IVideoEncoder() = default;
    virtual EncoderResult initialize(const VideoEncoderConfig& config) = 0;
    virtual EncoderResult encodeFrame(const VideoFrame& frame) = 0;
    virtual EncoderResult flush(std::vector<EncodedPacket>& out) = 0;
    virtual void          destroy() = 0;
    virtual EncoderType   type() const = 0;
    virtual std::string   name() const = 0;
    virtual EncoderStats  getStats() const = 0;
};
```

### ICapturer

```cpp
class ICapturer {
public:
    virtual ~ICapturer() = default;
    virtual CaptureResult initialize(const CaptureConfig& config) = 0;
    virtual void          start() = 0;
    virtual void          stop() = 0;
    virtual bool          getNextFrame(VideoFrame& out, uint32_t timeoutMs) = 0;
    virtual CaptureStats  getStats() const = 0;
};
```

### IRepository (Database)

```cpp
template<typename T>
class IRepository {
public:
    virtual std::string   insert(const T& entity) = 0;
    virtual std::optional<T> findById(const std::string& id) = 0;
    virtual bool          update(const T& entity) = 0;
    virtual bool          remove(const std::string& id) = 0;
    virtual std::vector<T> findAll(int limit = 50, int offset = 0) = 0;
};
```

---

## Error Codes

```c
// CSErrorCode
#define CSE_OK                  0
#define CSE_ERR_INVALID_CONFIG  1001
#define CSE_ERR_CAPTURE_INIT    1100
#define CSE_ERR_CAPTURE_LOST    1101
#define CSE_ERR_AUDIO_INIT      2000
#define CSE_ERR_AUDIO_DEVICE    2001
#define CSE_ERR_ENCODER_INIT    3000
#define CSE_ERR_ENCODER_FAIL    3001
#define CSE_ERR_DISK_FULL       4001
#define CSE_ERR_OUTPUT_PATH     4002
#define CSE_ERR_FATAL           9000
```

---

*Last updated: 2025 | Module 13 of 19*

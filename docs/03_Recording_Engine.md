# 03 — Recording Engine

## CodeStudio Recorder — Recording Engine

---

## Table of Contents

1. [Overview](#overview)
2. [Recording Lifecycle](#recording-lifecycle)
3. [Frame Pipeline](#frame-pipeline)
4. [Capture Controller](#capture-controller)
5. [Timing System](#timing-system)
6. [Synchronization](#synchronization)
7. [Frame Buffering](#frame-buffering)
8. [Recording State Machine](#recording-state-machine)
9. [Error Handling](#error-handling)
10. [Performance Budget](#performance-budget)

---

## Overview

The Recording Engine is the central coordinator of the entire capture-to-file pipeline. It owns the recording session lifecycle, manages all subordinate engines (Capture, Audio, Encoder), and enforces timing and synchronization contracts across the pipeline.

**Key responsibility:** Ensure that video frames and audio samples arrive at the Encoding Engine in the correct time order, at consistent intervals, with zero gaps or duplicates.

---

## Recording Lifecycle

```
[IDLE]
  │
  ▼ beginSession(config)
[INITIALIZING]
  │   - Validate config
  │   - Probe hardware encoders
  │   - Allocate frame buffers
  │   - Open audio devices
  │
  ▼ initComplete()
[READY]
  │
  ▼ startCapture()
[RECORDING]
  │   - Frame loop running
  │   - Audio loop running
  │   - Encoder consuming frames
  │
  ▼ pauseCapture() (optional)
[PAUSED]
  │
  ▼ resumeCapture()
[RECORDING]
  │
  ▼ stopCapture()
[FLUSHING]
  │   - Drain remaining frames
  │   - Flush encoder
  │   - Finalize container
  │
  ▼ flushComplete()
[FINALIZING]
  │
  ▼ fileWritten()
[COMPLETED]
  │
  ▼ reset()
[IDLE]
```

---

## Frame Pipeline

The frame pipeline moves raw GPU textures from the Capture Engine to the Encoding Engine in stages.

```
CaptureEngine (GPU texture)
       │
       ▼
[Stage 1] TextureCopyStage
  - Copy from GPU shared surface to encoding surface
  - Apply region crop if needed
  - Convert color space if required (e.g. BGRA → NV12)
       │
       ▼
[Stage 2] ProcessingStage (optional)
  - Cursor overlay rendering
  - Annotation overlay rendering
  - AI zoom application (Phase 4)
       │
       ▼
[Stage 3] FrameQueue (Lock-free SPSC ring buffer)
  - Decouples capture timing from encode timing
  - Capacity: 30 frames (configurable)
  - Drop policy: warn + skip oldest on overflow
       │
       ▼
[Stage 4] EncodingStage
  - Hardware encoder consumes from FrameQueue
  - Produces compressed NAL units / packets
       │
       ▼
[Stage 5] MuxStage
  - Interleaves video packets with audio packets
  - Writes to container file
```

### Frame Object

```cpp
struct VideoFrame {
    ID3D11Texture2D* texture;     // GPU texture (borrowed reference)
    int64_t          captureTime; // QPC timestamp (100ns ticks)
    uint32_t         width;
    uint32_t         height;
    DXGI_FORMAT      format;
    FrameFlags       flags;       // keyframe hint, drop hint, etc.
};
```

---

## Capture Controller

The `CaptureController` is responsible for selecting and initializing the correct capture backend based on the user's selection and the target window/screen.

```cpp
class CaptureController {
public:
    CaptureResult initialize(CaptureConfig config);
    void          start();
    void          stop();
    void          setRegion(RECT region);
    CaptureStats  getStats() const;

private:
    std::unique_ptr<ICapturer> capturer_; // WGC or DXGI backend
    FrameQueue                 frameQueue_;
    CaptureThread              captureThread_;
};
```

### Capture Backend Selection

```
Target is a window?
  └─ WGC (Windows.Graphics.Capture) ← preferred, works with most apps
  
Target is full screen / monitor?
  └─ WGC (monitor capture)
  
WGC not available (older Windows)?
  └─ DXGI Desktop Duplication API (fallback)
  
DXGI not available?
  └─ GDI BitBlt (last resort, limited to 30fps, no HDR)
```

---

## Timing System

Precise timestamps are required to achieve smooth playback and proper A/V sync.

### Clock Source

All timestamps use **Windows Query Performance Counter (QPC)**:

```cpp
LARGE_INTEGER captureTime;
QueryPerformanceCounter(&captureTime);
frame.captureTime = captureTime.QuadPart;
```

QPC resolution is typically <100ns on modern Windows systems.

### Frame Rate Control

The Capture Engine produces frames at the screen's refresh rate (60Hz, 120Hz, 144Hz, etc.). The Recording Engine applies **frame rate limiting** if the target output FPS is lower:

```
Screen = 144Hz → target = 60fps
FrameRateLimiter: keep every N-th frame
N = floor(144 / 60) = 2 → keep every 2nd frame
```

If the encoder falls behind:
- Warning logged
- Frame dropped (oldest frame in queue discarded)
- Drop counter incremented (visible in stats)

### Presentation Time

PTS (Presentation Timestamp) for each frame is computed relative to the session start time:

```cpp
int64_t pts = (frame.captureTime - session_.startTime) 
              * encoderTimeBase_ / qpcFrequency_;
```

---

## Synchronization

Audio and video synchronization is maintained through a shared `SessionClock`.

### SessionClock

```cpp
class SessionClock {
public:
    void        start();
    void        pause();
    void        resume();
    int64_t     now() const; // microseconds since session start
    int64_t     pausedDuration() const;
};
```

Both `CaptureEngine` and `AudioEngine` stamp their output with `SessionClock::now()` timestamps. The `Muxer` uses these timestamps to interleave frames and audio packets correctly.

### Drift Compensation

Over long recordings, minor drift between video and audio clocks can accumulate. The engine monitors A/V offset every 10 seconds and applies correction:

```
Δ = videoClock - audioClock
If |Δ| > 40ms:
  → Insert silence (audio behind) or duplicate frame (video behind)
  → Log warning: "A/V drift corrected: +/- Δms at T seconds"
```

---

## Frame Buffering

### Texture Pool

GPU textures are expensive to allocate. CodeStudio uses a pre-allocated `TexturePool` to eliminate per-frame allocation overhead.

```cpp
class TexturePool {
public:
    explicit TexturePool(ID3D11Device* device, size_t capacity);
    
    TextureHandle acquire();   // blocks if pool exhausted
    void          release(TextureHandle);

private:
    std::vector<ComPtr<ID3D11Texture2D>> textures_;
    SPSCQueue<TextureHandle>             available_;
};
```

Default pool size: **8 textures** (configurable). Each texture is sized to the capture resolution.

### Frame Queue

```cpp
template<typename T, size_t Capacity>
class SPSCRingBuffer {
    // Lock-free single-producer single-consumer ring buffer
    // Used between CaptureThread → EncodeThread
    
    bool push(T item);   // called by capture thread
    bool pop(T& item);   // called by encode thread
    size_t size() const;
};
```

Capacity defaults to **30 frames** (~500ms at 60fps). If the queue fills:
1. Oldest frame is released back to TexturePool
2. Drop counter increments
3. Warning emitted if drops exceed 5% of total frames

---

## Recording State Machine

```cpp
enum class RecordingState {
    Idle,
    Initializing,
    Ready,
    Recording,
    Paused,
    Flushing,
    Finalizing,
    Completed,
    Error
};

class RecordingStateMachine {
public:
    bool transition(RecordingEvent event);
    RecordingState current() const;

private:
    static const TransitionTable kTransitions;
};
```

### Transition Table

| Current State | Event | Next State |
|---|---|---|
| Idle | Initialize | Initializing |
| Initializing | InitOK | Ready |
| Initializing | InitFail | Error |
| Ready | Start | Recording |
| Recording | Pause | Paused |
| Recording | Stop | Flushing |
| Paused | Resume | Recording |
| Paused | Stop | Flushing |
| Flushing | FlushDone | Finalizing |
| Finalizing | FileDone | Completed |
| Any | FatalError | Error |
| Error | Reset | Idle |
| Completed | Reset | Idle |

---

## Error Handling

### Error Categories

| Error | Recovery Strategy |
|---|---|
| Capture device lost | Attempt re-initialize once; fail to Error state |
| Encoder overload (frames dropped) | Log warning; continue recording |
| Audio device disconnected | Mute audio track; continue video-only |
| Disk full | Pause recording; notify user immediately |
| Encoder hardware failure | Fall back to software encoder |
| Fatal / unknown | Flush what is possible; save partial file |

### Partial Save on Fatal Error

If a fatal error occurs mid-recording:

```
RecordingEngine receives FatalError event
  → Stop accepting new frames
  → Flush encoder with what is buffered
  → Finalize container (may be truncated)
  → Save partial file as: output_PARTIAL_<timestamp>.mp4
  → Notify UI with partial save path
```

---

## Performance Budget

| Operation | Budget |
|---|---|
| Frame capture latency | < 2ms per frame |
| Texture copy (GPU → encode surface) | < 1ms per frame |
| Frame queue push | < 1μs (lock-free) |
| Audio capture latency | < 5ms per buffer |
| Session start time | < 300ms from user trigger |
| Stop + flush time | < 2 seconds |

All budgets are measured at 1080p60 on mid-range hardware.

---

*Last updated: 2025 | Module 03 of 19*

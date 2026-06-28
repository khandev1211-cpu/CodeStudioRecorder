# 02 — System Architecture

## CodeStudio Recorder — Complete Software Architecture

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Layer Definitions](#layer-definitions)
3. [Communication Flow](#communication-flow)
4. [Design Principles](#design-principles)
5. [Dependency Graph](#dependency-graph)
6. [Component Diagrams](#component-diagrams)
7. [Threading Model](#threading-model)
8. [IPC Bridge (FFI)](#ipc-bridge-ffi)
9. [Error Propagation](#error-propagation)
10. [Scalability Considerations](#scalability-considerations)

---

## Architecture Overview

CodeStudio Recorder is built on a **strict layered architecture** with clear separation between:

- The **Flutter UI layer** — declarative, reactive, platform-agnostic
- The **Application Controller** — orchestration and session management
- The **Native Engine** — all performance-critical paths in C++
- The **Export Pipeline** — encoding, muxing, and file output

```
┌─────────────────────────────────────────────┐
│              Flutter UI Layer               │
│  (Dart — widgets, state, user interaction)  │
└─────────────────┬───────────────────────────┘
                  │ FFI / Platform Channel
┌─────────────────▼───────────────────────────┐
│           Application Controller            │
│   (C++ — session, config, lifecycle mgmt)   │
└──────┬────────────┬─────────────────────────┘
       │            │
┌──────▼──────┐ ┌───▼──────────┐
│  Recording  │ │   Database   │
│   Engine    │ │   (SQLite)   │
└──────┬──────┘ └──────────────┘
       │
┌──────▼──────────────────────────────────────┐
│              Capture Engine                 │
│  (Windows Graphics Capture API, DXGI, D3D)  │
└──────┬──────────────────────────────────────┘
       │
┌──────▼──────────────────────────────────────┐
│              Audio Engine                   │
│         (WASAPI — mic + system audio)        │
└──────┬──────────────────────────────────────┘
       │
┌──────▼──────────────────────────────────────┐
│             Encoding Engine                 │
│   (FFmpeg + NVENC / Quick Sync / AMF / SW)  │
└──────┬──────────────────────────────────────┘
       │
┌──────▼──────────────────────────────────────┐
│             Export Pipeline                 │
│     (Muxing, container format, metadata)     │
└──────┬──────────────────────────────────────┘
       │
┌──────▼──────────────────────────────────────┐
│              Output File                    │
│        (.mp4, .mkv, .webm, .gif, ...)        │
└─────────────────────────────────────────────┘
```

---

## Layer Definitions

### Layer 1 — Flutter UI
- **Language:** Dart
- **Framework:** Flutter Desktop (Windows)
- **Responsibility:** All visual rendering, user input, navigation, animation, state management
- **Communicates with:** Application Controller via FFI bridge
- **Must NOT:** Access native Windows APIs directly, perform heavy computation, block the UI thread

### Layer 2 — Application Controller
- **Language:** C++
- **Responsibility:** Central orchestration — coordinates all subsystems, manages recording sessions, owns configuration state
- **Communicates with:** All engine layers downward; Flutter UI upward via FFI
- **Key classes:** `SessionManager`, `ConfigManager`, `PluginHost`

### Layer 3 — Recording Engine
- **Language:** C++
- **Responsibility:** Frame pipeline coordination, timing, synchronization, recording state machine
- **Communicates with:** Capture Engine, Audio Engine, Encoding Engine
- **Key classes:** `RecordingSession`, `FramePipeline`, `TimingController`, `StateManager`

### Layer 4 — Capture Engine
- **Language:** C++ with WinRT
- **Responsibility:** Screen frame capture via Windows APIs; DXGI texture acquisition
- **APIs used:** `Windows.Graphics.Capture`, `IDXGIOutputDuplication`, Direct3D 11
- **Key classes:** `ScreenCapturer`, `WindowCapturer`, `RegionCapturer`, `TexturePool`

### Layer 5 — Audio Engine
- **Language:** C++ with WASAPI
- **Responsibility:** Microphone capture, system audio loopback capture, mixing, sync
- **APIs used:** WASAPI (`IAudioClient`, `IAudioCaptureClient`)
- **Key classes:** `MicCapture`, `LoopbackCapture`, `AudioMixer`, `SyncClock`

### Layer 6 — Encoding Engine
- **Language:** C++ + FFmpeg
- **Responsibility:** Hardware/software video and audio encoding, bitrate management
- **Encoders:** NVENC (NVIDIA), Quick Sync (Intel), AMF (AMD), libx264 (software fallback)
- **Key classes:** `EncoderFactory`, `VideoEncoder`, `AudioEncoder`, `BitrateController`

### Layer 7 — Export Pipeline
- **Language:** C++ + FFmpeg
- **Responsibility:** Muxing audio + video streams, writing container format, metadata injection
- **Key classes:** `Muxer`, `FormatWriter`, `MetadataInjector`

---

## Communication Flow

### Recording Start Sequence

```
User presses Record (Flutter UI)
        ↓
Dart calls FFI: startRecording(config)
        ↓
Application Controller → SessionManager.beginSession()
        ↓
SessionManager → RecordingEngine.start()
        ↓
RecordingEngine → CaptureEngine.startCapture()
                → AudioEngine.startCapture()
                → EncoderEngine.initialize()
        ↓
CaptureEngine begins frame loop (dedicated thread)
AudioEngine begins audio loop (dedicated thread)
        ↓
Frames → FramePipeline → EncoderEngine → Muxer → File
```

### Recording Stop Sequence

```
User presses Stop (Flutter UI)
        ↓
Dart calls FFI: stopRecording()
        ↓
SessionManager.endSession()
        ↓
RecordingEngine.stop()
        → CaptureEngine.stopCapture() — drain remaining frames
        → AudioEngine.stopCapture() — drain audio buffer
        → EncoderEngine.flush() — flush encoder queues
        ↓
ExportPipeline.finalize() — write final container
        ↓
File closed → FFI callback → Flutter UI notification
```

---

## Design Principles

### 1. Separation of Concerns
No engine layer should reach across to another engine layer directly. All cross-engine communication routes through the Application Controller or Recording Engine coordinator.

### 2. Thread Isolation
Each engine component runs on its own dedicated thread or thread pool. No shared mutable state without explicit synchronization.

### 3. Lock-Free Frame Queues
The frame pipeline uses lock-free ring buffers (SPSC — Single Producer Single Consumer) to transfer frames between capture, processing, and encoding stages without mutex contention.

### 4. No UI on Native Thread
The native C++ layer never touches Flutter UI. All UI updates are sent via FFI callbacks to Dart.

### 5. Plugin Isolation
Plugins run in a sandboxed host and access the engine only through the Plugin SDK interface. They cannot directly access engine internals.

### 6. Fail-Safe First
If encoding fails, capture continues and frames are buffered. The user is notified rather than the recording silently corrupting.

---

## Dependency Graph

```
Flutter UI
  └── depends on → Application Controller (FFI)

Application Controller
  ├── depends on → Recording Engine
  ├── depends on → Database (SQLite)
  └── depends on → Plugin Host

Recording Engine
  ├── depends on → Capture Engine
  ├── depends on → Audio Engine
  └── depends on → Encoding Engine

Encoding Engine
  └── depends on → Export Pipeline

Capture Engine
  └── depends on → Windows Graphics Capture, D3D11, DXGI

Audio Engine
  └── depends on → WASAPI

Export Pipeline
  └── depends on → FFmpeg libavformat, libavcodec
```

No upward dependencies exist. Lower layers never depend on higher layers.

---

## Component Diagrams

### Capture Engine Internals

```
ScreenCapturer
  ├── WGCSession (Windows.Graphics.Capture session)
  ├── D3D11Device (shared device)
  ├── TexturePool (pre-allocated GPU texture ring)
  ├── FrameQueue (lock-free SPSC queue)
  └── CaptureThread (dedicated high-priority thread)
```

### Audio Engine Internals

```
AudioEngine
  ├── MicCapture (IAudioCaptureClient — exclusive mode)
  ├── LoopbackCapture (IAudioCaptureClient — loopback)
  ├── AudioMixer (float PCM mixing)
  ├── ResampleFilter (if sample rate mismatch)
  └── AudioQueue (lock-free SPSC queue)
```

### Encoding Engine Internals

```
EncoderFactory
  ├── probeHardware() → selects best available encoder
  ├── NVENCEncoder (NVIDIA GPU encoder)
  ├── QuickSyncEncoder (Intel GPU encoder)
  ├── AMFEncoder (AMD GPU encoder)
  └── SoftwareEncoder (libx264 / libx265 fallback)
```

---

## Threading Model

| Thread | Owned By | Priority | Purpose |
|---|---|---|---|
| UI Thread | Flutter | Normal | Widget rendering, user input |
| Capture Thread | CaptureEngine | High | Frame acquisition loop |
| Audio Thread | AudioEngine | High | Audio sample capture loop |
| Encode Thread | EncoderEngine | Above Normal | Frame encoding |
| Write Thread | ExportPipeline | Normal | Disk I/O (muxed output) |
| DB Thread | Database | Below Normal | SQLite reads/writes |
| Plugin Thread | PluginHost | Normal | Plugin execution isolation |

All inter-thread communication uses lock-free queues or event handles. Mutexes are avoided in the hot path (capture → encode).

---

## IPC Bridge (FFI)

Flutter communicates with native C++ through Dart FFI (Foreign Function Interface):

```
// Dart side
final _lib = DynamicLibrary.open('codestudio_engine.dll');

final startRecording = _lib.lookupFunction<
  Int32 Function(Pointer<RecordingConfig>),
  int Function(Pointer<RecordingConfig>)
>('cse_start_recording');
```

```cpp
// C++ side (exported)
extern "C" __declspec(dllexport)
int32_t cse_start_recording(RecordingConfig* config) {
    return SessionManager::instance().beginSession(*config);
}
```

### FFI Contract Rules
- All structs passed across FFI boundary are plain C structs (no C++ objects)
- Strings use `const char*` with explicit length parameters
- Callbacks from native → Dart use `NativeCallable` (Dart 3+)
- No exceptions cross the FFI boundary — only error codes

---

## Error Propagation

```
Native error → Error code (int32_t) → FFI return value
→ Dart receives code → ErrorMapper.toException()
→ Flutter UI shows appropriate feedback
```

### Error Code Ranges

| Range | Category |
|---|---|
| 0 | Success |
| 1000–1999 | Capture errors |
| 2000–2999 | Audio errors |
| 3000–3999 | Encoder errors |
| 4000–4999 | Export/mux errors |
| 5000–5999 | Configuration errors |
| 9000+ | Critical / unrecoverable |

---

## Scalability Considerations

- **Plugin host** is designed to support unlimited plugins without core engine changes
- **Encoder factory** can register new hardware encoders as new GPU vendors are supported
- **Frame pipeline** supports inserting processing stages (AI zoom, annotation overlay) without breaking existing stages
- **Multi-monitor** capture is handled by instantiating parallel `ScreenCapturer` instances
- **Database layer** is schema-versioned for safe future migrations without data loss

---

*Last updated: 2025 | Module 02 of 19*

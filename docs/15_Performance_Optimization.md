# 15 — Performance Optimization

## CodeStudio Recorder — Performance & Optimization

---

## Overview

Performance is a first-class citizen in CodeStudio Recorder. This document defines memory optimization strategies, CPU optimization targets, GPU optimization approaches, rendering performance guidelines, profiling tools, and benchmarking targets.

---

## Performance Targets

| Metric | Target | Measurement |
|---|---|---|
| App startup time | < 2 seconds | Cold launch to ready state |
| Recording start latency | < 300ms | Button press to first frame captured |
| Recording stop + save | < 2 seconds | Stop press to file complete |
| UI frame rate | 60fps constant | Flutter rendering thread |
| Encoder frame time | < 2ms per frame | NVENC at 1080p60 |
| Audio latency | < 10ms | Mic input to encoded packet |
| CPU usage (idle) | < 1% | App open, not recording |
| CPU usage (recording 1080p60) | < 5% | With hardware encoding |
| RAM usage (idle) | < 80MB | App open, no recording active |
| RAM usage (recording) | < 150MB | 1080p60, hardware encoding |
| GPU VRAM usage | < 128MB | Texture pool + encoding surfaces |

---

## Memory Optimization

### Texture Pool

Pre-allocate a fixed number of GPU textures at session start to avoid per-frame allocation:

```cpp
// Allocate 8 textures at 1920×1080 BGRA = ~63MB VRAM for the pool
TexturePool pool(device, capacity: 8, width: 1920, height: 1080);
```

**Never allocate GPU resources on the hot path (per-frame).**

### Zero-Copy Frame Pipeline

Frames flow from capture → processing → encoding **without CPU copies**:

1. WGC gives a shared GPU texture handle
2. CaptureEngine maps it to our texture pool slot (GPU ↔ GPU copy via D3D)
3. EncoderEngine reads from the same GPU surface — no CPU round-trip

### Dart Object Allocation

Minimize Dart object allocations in hot paths (recording stats polling):

```dart
// Bad: allocates new RecordingStats every 100ms
final stats = RecordingStats(fps: nativeFps, ...);

// Good: update in-place / use cached object
statsCache.fps = nativeFps; // update primitive fields only
```

Use `@pragma('vm:prefer-inline')` on hot Dart FFI call wrappers.

---

## CPU Optimization

### Lock-Free Queues

All inter-thread communication in the hot path uses SPSC ring buffers — no mutex contention:

```
CaptureThread → SPSCQueue → EncodeThread
AudioThread   → SPSCQueue → MixerThread → SPSCQueue → EncodeThread
```

### Thread Affinity

Critical threads pinned to performance cores on systems with hybrid CPU topology (e.g. Intel 12th Gen):

```cpp
// Pin capture thread to P-core 0
SetThreadAffinityMask(captureThread_.native_handle(), 0x01);
```

### SIMD Audio Mixing

The AudioMixer uses SSE2/AVX2 SIMD for float32 PCM mixing:

```cpp
// Mix 8 float32 samples per iteration with AVX2
void mixAVX2(float* dst, const float* src, float gain, size_t count) {
    __m256 gainVec = _mm256_set1_ps(gain);
    for (size_t i = 0; i < count; i += 8) {
        __m256 s = _mm256_loadu_ps(src + i);
        __m256 d = _mm256_loadu_ps(dst + i);
        _mm256_storeu_ps(dst + i, _mm256_add_ps(d, _mm256_mul_ps(s, gainVec)));
    }
}
```

### Encoder Selection

Hardware encoders (NVENC/QSV/AMF) use offload encoding to the GPU, freeing CPU for other work. The `EncoderFactory` always prefers hardware over software.

---

## GPU Optimization

### Shared GPU Surface

WGC captures directly to a `ID3D11Texture2D` on the GPU. The encoder reads from the same GPU memory — no CPU involvement:

```
[Screen pixels on GPU] → [WGC texture] → [NVENC encoder input] → [Compressed bitstream]
```

### Direct3D Device Sharing

The capture engine and NVENC encoder share the same `ID3D11Device` and `ID3D11DeviceContext` to avoid cross-device texture copies.

### Compute Shader for Zoom

The ZoomStage uses a Direct3D 11 compute shader for hardware-accelerated bilinear zoom — no CPU scaling:

```hlsl
// zoom.hlsl
[numthreads(8, 8, 1)]
void ZoomCS(uint3 id : SV_DispatchThreadID) {
    float2 uv = (float2(id.xy) / float2(dstSize)) * zoomScale + zoomOffset;
    output[id.xy] = input.SampleLevel(linearSampler, uv, 0);
}
```

---

## Flutter Rendering Performance

### Const Widgets

Mark all stateless, non-rebuilding widgets as `const`:

```dart
const CsButton(label: 'Record', onPressed: null)  // Good
CsButton(label: 'Record', onPressed: null)         // Allocates every build
```

### RepaintBoundary

Wrap heavy, frequently-updating widgets in `RepaintBoundary` to isolate repaints:

```dart
RepaintBoundary(
  child: CsAudioMeter(source: AudioSource.mic),
)
```

### Avoid Rebuilding on Every Stat Poll

Use `select` to subscribe to only the fields that matter:

```dart
// Bad: rebuilds entire widget on any RecordingState change
ref.watch(recordingStateProvider)

// Good: only rebuilds when fps changes
final fps = ref.watch(
  recordingStateProvider.select((s) => s.currentFps)
);
```

---

## Profiling Tools

### Native Engine Profiling

| Tool | Purpose |
|---|---|
| Visual Studio Performance Profiler | CPU sampling, call tree |
| PIX for Windows | GPU timing, frame analysis |
| WPA (Windows Performance Analyzer) | System-wide CPU, disk, memory |
| NVIDIA Nsight | NVENC utilization, GPU timeline |
| ETW (Event Tracing for Windows) | Custom instrumentation |

### Flutter Profiling

```bash
# Run with performance overlay
flutter run --profile -d windows

# Timeline tracing
flutter run --trace-startup -d windows
```

Use Flutter DevTools for:
- Frame timeline (detect jank)
- Memory allocation timeline
- CPU profiler

---

## Benchmarking

### Automated Benchmark Suite

```cpp
class RecordingBenchmark {
    void BM_FrameCapture(benchmark::State& state) {
        auto capturer = createWindowsCapturer();
        capturer->start();
        for (auto _ : state) {
            VideoFrame frame;
            capturer->getNextFrame(frame, 100);
            capturer->releaseFrame(frame);
        }
    }
    BENCHMARK(BM_FrameCapture)->Unit(benchmark::kMicrosecond);
    
    void BM_NVENCEncode1080p60(benchmark::State& state) {
        auto encoder = createNVENCEncoder(1920, 1080, 60);
        auto frame   = createTestFrame(1920, 1080);
        for (auto _ : state) {
            encoder->encodeFrame(frame);
        }
    }
    BENCHMARK(BM_NVENCEncode1080p60)->Unit(benchmark::kMicrosecond);
};
```

Benchmarks run nightly in CI. A 15% regression in any benchmark blocks the release pipeline.

---

*Last updated: 2025 | Module 15 of 19*

# 06 — AI Architecture

## CodeStudio Recorder — AI Architecture

---

## Table of Contents

1. [Overview](#overview)
2. [AI Roadmap](#ai-roadmap)
3. [Smart Zoom (Cursor Tracking)](#smart-zoom-cursor-tracking)
4. [Automatic Captions](#automatic-captions)
5. [Silence Detection](#silence-detection)
6. [Highlight Generation](#highlight-generation)
7. [Background Noise Removal](#background-noise-removal)
8. [AI Pipeline Architecture](#ai-pipeline-architecture)
9. [Model Strategy](#model-strategy)
10. [Future AI Modules](#future-ai-modules)

---

## Overview

CodeStudio Recorder is designed from day one to be **AI-native** — meaning the architecture anticipates AI processing stages in the frame pipeline, audio pipeline, and post-processing pipeline without requiring fundamental refactoring.

AI features are introduced in **Phase 4** but are architecturally provisioned in Phase 1 so that their insertion does not require breaking changes.

---

## AI Roadmap

| Feature | Phase | Type | Model |
|---|---|---|---|
| Smart Zoom | 4 | Real-time | Custom CNN / OpenCV |
| Cursor-following pan | 4 | Real-time | Heuristic + ML |
| Auto Captions | 4 | Near real-time | Whisper (local) |
| Silence Detection | 4 | Real-time | VAD model |
| Highlight Generation | 4 | Post-process | LLM + activity heuristics |
| Noise Removal | 4 | Real-time | RNNoise / DeepFilterNet |
| Scene Detection | 4 | Post-process | CV model |
| Auto Chapter Markers | 5 | Post-process | LLM + scene model |
| Avatar / Webcam BG Remove | 5 | Real-time | Selfie segmentation model |

---

## Smart Zoom (Cursor Tracking)

### Goal

Automatically zoom into the area of the screen where the user is actively working — making coding tutorials and demos dramatically more readable without manual zoom cuts.

### Approach

**Heuristic Layer (always active):**
- Track cursor position via `GetCursorPos()` at capture rate
- Apply smoothed pan to keep cursor in the center third of the frame
- Zoom level: 1.5× to 2.5× (configurable)
- Smoothing: exponential moving average with configurable lag

**AI Enhancement Layer (Phase 4):**
- Lightweight attention model detects "active zone" (where the user is typing, clicking)
- Inputs: cursor position, click events, keyboard activity signal
- Output: smooth (x, y, scale) target for the zoom viewport
- Model: ONNX runtime, custom-trained on developer screen recordings

### Zoom Stage in Frame Pipeline

```
CaptureEngine → [ZoomStage] → EncodingEngine
                     │
               ZoomController
                 - currentZoom (0.0 – 1.0)
                 - viewportCenter (x, y)
                 - smoothingFactor
```

The `ZoomStage` is inserted as a `ProcessingStage` in the frame pipeline. It operates on GPU textures using Direct3D compute shaders for zero-copy, hardware-accelerated zoom.

```cpp
class ZoomStage : public IFrameProcessor {
public:
    void setZoomTarget(float zoom, float cx, float cy);
    void process(VideoFrame& frame) override;

private:
    ID3D11ComputeShader* zoomShader_;
    ZoomController       controller_;
};
```

---

## Automatic Captions

### Goal

Generate real-time closed captions / subtitles from microphone audio, burned into the video or exported as a separate `.srt` / `.vtt` file.

### Approach

**Model:** OpenAI Whisper (local, ONNX runtime)

**Deployment:**
- Whisper `tiny` or `base` model for real-time (runs on CPU or GPU)
- Whisper `medium` for post-processing high-quality transcription

```
AudioEngine → [CaptionBuffer] → WhisperInference → CaptionRenderer
```

### CaptionBuffer

Buffers 2–5 seconds of audio (depending on model size), then sends to inference:

```cpp
class CaptionBuffer {
    std::deque<AudioFrame> buffer_;
    size_t                 windowSeconds_ = 3;
    
    void push(AudioFrame frame);
    std::vector<float> getWindow(); // returns float32 mel spectrogram
};
```

### WhisperInference

```cpp
class WhisperInference {
public:
    bool initialize(const std::string& modelPath);
    std::string transcribe(const std::vector<float>& melSpec);

private:
    Ort::Session onnxSession_;
};
```

### Caption Renderer

Burns captions into frames using Direct2D overlay on the capture surface:

```cpp
class CaptionRenderer {
    ID2D1RenderTarget* d2dTarget_;
    IDWriteTextFormat* textFormat_;
    
    void renderCaption(const std::string& text, VideoFrame& frame);
};
```

**Output formats:**
- Burned-in (baked into video)
- `.srt` (SubRip, offline)
- `.vtt` (WebVTT, web-friendly)

---

## Silence Detection

### Goal

Automatically detect and optionally remove silent/dead-air segments from recordings (pauses while the presenter is thinking, mouse moving with no speech, etc.).

### Approach

**VAD (Voice Activity Detection) model:**
- WebRTC VAD (lightweight C library, MIT license)
- Or: Silero VAD (ONNX, more accurate, slightly heavier)

```cpp
class SilenceDetector {
public:
    void     initialize(VADModel model = VADModel::WebRTC);
    bool     isSpeech(const AudioBuffer& buffer);
    SilenceEvent detectSilenceSegments(const std::string& audioPath);
};
```

### Silence Marking

During recording, silence segments are **marked in the timeline** rather than cut immediately. The user can review and remove them in post-processing (Phase 2 editor) or enable auto-cut on export.

```cpp
struct SilenceMarker {
    int64_t startMs;
    int64_t endMs;
    float   confidenceScore; // 0.0 – 1.0
};
```

---

## Highlight Generation

### Goal

Automatically identify the most interesting segments of a long recording for clip generation (YouTube Shorts, Reels, highlight reels).

### Approach

**Signal fusion:**
| Signal | Weight | Description |
|---|---|---|
| Audio energy | 0.25 | Loud, animated speech = high energy |
| Typing activity | 0.20 | Active coding |
| Click density | 0.15 | Rapid interaction |
| Cursor velocity | 0.10 | Fast movement = active use |
| Speech confidence | 0.30 | Clear speech from VAD model |

**Scoring:**
```
highlightScore(t) = Σ(signal_i(t) × weight_i)
```

Segments above a threshold (`>= 0.65`) are marked as candidate highlights.

**LLM Post-Filter (Phase 4):**
Candidate segments are scored by an LLM (running locally or via API) for context quality:
- "Is this a complete thought / demo?"
- "Is the code visible and legible?"
- "Is there audio commentary?"

Output: ordered list of `{start, end, score, reason}` highlight clips.

---

## Background Noise Removal

### Goal

Remove background noise from microphone audio in real-time (fans, keyboard, traffic, etc.).

### Models

**Option A: RNNoise (C library)**
- Extremely low CPU footprint
- Designed for real-time speech enhancement
- Good for mild noise (fan, AC)

**Option B: DeepFilterNet (ONNX)**
- Higher quality noise suppression
- Handles music, traffic, crowd noise
- Runs on CPU ~2–5ms per 10ms frame

```cpp
class NoiseRemover {
public:
    bool initialize(NoiseModel model);
    void process(AudioBuffer& buf); // in-place

private:
    // Either RNNoise state or ONNX session
    rnnoise_state_t*  rnnoiseState_;
    Ort::Session      onnxSession_;
    NoiseModel        activeModel_;
};
```

The `NoiseRemover` is inserted into the audio pipeline between `MicCapture` and `AudioMixer`.

---

## AI Pipeline Architecture

### Frame Pipeline with AI Stages

```
CaptureEngine
     │
[ZoomStage]          ← AI cursor tracking
     │
[AnnotationStage]    ← Captions overlay (if burn-in)
     │
[EncodingEngine]
```

### Audio Pipeline with AI Stages

```
MicCapture
     │
[NoiseRemover]       ← DeepFilterNet / RNNoise
     │
[SilenceDetector]    ← VAD marking
     │
[AudioMixer]
     │
[CaptionBuffer] ──→ [WhisperInference] ──→ [CaptionRenderer]
     │
[AudioEncoder]
```

### AI Stage Interface

All AI stages implement a common interface:

```cpp
class IAIStage {
public:
    virtual bool initialize(const AIStageConfig& config) = 0;
    virtual void process(DataFrame& frame) = 0; // in-place
    virtual AIStageStats getStats() const = 0;
    virtual ~IAIStage() = default;
};
```

---

## Model Strategy

### Deployment Philosophy

- **Local-first**: All AI models run on-device. No audio/video data leaves the machine.
- **ONNX Runtime**: All ML models are deployed as ONNX for platform portability and optimized inference.
- **DirectML**: GPU-accelerated inference on Windows via DirectML (works on NVIDIA, AMD, Intel GPUs).

### Model Size Targets

| Model | Size | Inference Time | Deployment |
|---|---|---|---|
| Whisper tiny | ~39MB | ~50ms / 3s audio | Bundled |
| Silero VAD | ~1.8MB | <1ms / 30ms audio | Bundled |
| RNNoise | ~100KB | <0.5ms / 10ms | Bundled |
| DeepFilterNet | ~36MB | ~3ms / 10ms | Optional download |
| Smart Zoom CNN | ~5MB | ~2ms / frame | Bundled |

---

## Future AI Modules

| Module | Phase | Description |
|---|---|---|
| Auto chapter markers | 5 | Scene change detection + LLM topic labeling |
| Presenter webcam BG removal | 5 | Selfie segmentation, virtual background |
| Code OCR enhancement | 5 | AI sharpening of code text in recordings |
| Smart crop for Shorts | 5 | Auto-center crop from widescreen to 9:16 |
| Voice clone dubbing | Future | Translate + revoice tutorials in other languages |
| AI narration | Future | Auto-generate voiceover from on-screen code |

---

*Last updated: 2025 | Module 06 of 19*

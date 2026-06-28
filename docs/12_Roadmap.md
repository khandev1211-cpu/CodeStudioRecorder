# 12 — Roadmap

## CodeStudio Recorder — Development Roadmap

---

## Overview

CodeStudio Recorder development is organized into 5 phases, each building on the previous. This document defines milestones, deliverables, and success criteria per phase.

---

## Phase 1 — Core Recorder (MVP)

**Goal:** Ship a working, performant screen recorder with hardware encoding.

### Milestones

| # | Milestone | Description |
|---|---|---|
| 1.1 | Engine Foundation | [DONE] CMake setup, FFI bridge, DLL export structure |
| 1.2 | Capture Engine | [DONE] WGC window/monitor capture, TexturePool, FrameQueue |
| 1.3 | Audio Engine | [DONE] WASAPI system audio loopback capture |
| 1.4 | Encoding Engine | [IN PROGRESS] FFmpeg encoder structure, EncoderFactory |
| 1.5 | Flutter UI Shell | [DONE] Navigation, Home, History, Settings screens |
| 1.6 | Recording Flow | [DONE] Start/stop/pause via UI and hotkeys |
| 1.7 | History Screen | [DONE] List recordings, persist to JSON |
| 1.8 | Settings System | [DONE] Persistent settings (C++ backend + Flutter UI) |
| 1.9 | Installer | NSIS installer, basic auto-update check |
| 1.10 | Alpha Release | Internal testing build |

### MVP Definition

A Phase 1 MVP ships when:
- ✅ User can record screen (window or full-screen) to MP4
- ✅ Microphone and system audio are captured and mixed
- ✅ NVENC or Quick Sync hardware encoding works
- ✅ Recording appears in history
- ✅ Settings persist across app restarts
- ✅ App starts in under 2 seconds
- ✅ Recording starts in under 500ms

---

## Phase 2 — Professional Creator Tools

**Goal:** Add creator-focused tools that differentiate from basic recorders.

### Milestones

| # | Milestone | Description |
|---|---|---|
| 2.1 | Cursor Effects | Cursor highlight, click animation, ripple effects |
| 2.2 | Zoom on Cursor | Heuristic zoom following active area |
| 2.3 | Annotation Layer | Draw arrows, boxes, text on recording |
| 2.4 | Export Presets | YouTube, Shorts, Reels, Twitter presets |
| 2.5 | Clip Trimmer | Simple in/out trim before export |
| 2.6 | Chapter Markers | Manual markers during recording |
| 2.7 | Webcam Overlay | PiP webcam from capture device |
| 2.8 | Audio Cleanup | Noise gate, high-pass filter, normalization |
| 2.9 | Recording Profiles | Save/load named recording configurations |
| 2.10 | Thumbnail Generator | Auto or custom thumbnail selection |

---

## Phase 3 — Plugin Ecosystem

**Goal:** Open CodeStudio to third-party extensions.

### Milestones

| # | Milestone | Description |
|---|---|---|
| 3.1 | Plugin SDK v1 | C++ SDK, DLL loading, permission model |
| 3.2 | Plugin Host | Sandboxed process for plugin execution |
| 3.3 | Plugin Manager UI | Install, enable, disable, configure plugins |
| 3.4 | SDK Documentation | Full SDK reference + sample plugins |
| 3.5 | Sample Plugins | Watermark, keystroke display, FPS counter |
| 3.6 | Plugin Marketplace | Web directory of community plugins |

---

## Phase 4 — AI Features

**Goal:** Integrate AI to reduce editing work and improve output quality.

### Milestones

| # | Milestone | Description |
|---|---|---|
| 4.1 | AI Smart Zoom | ML-powered cursor tracking zoom |
| 4.2 | Noise Removal | RNNoise / DeepFilterNet in audio pipeline |
| 4.3 | Silence Detection | VAD-based silence marking + auto-cut |
| 4.4 | Auto Captions | Whisper local transcription, burn-in + .srt |
| 4.5 | Highlight Detection | Signal-fusion highlight scoring |
| 4.6 | AI Export Assistant | Suggests best export preset based on content |
| 4.7 | Auto Chapter Markers | Scene change detection + LLM labeling |

---

## Phase 5 — Streaming Platform

**Goal:** Add live streaming capabilities.

### Milestones

| # | Milestone | Description |
|---|---|---|
| 5.1 | RTMP Output | Stream to YouTube, Twitch via RTMP |
| 5.2 | Scene Management | Multiple scenes with layout management |
| 5.3 | Live Overlays | Real-time graphic overlays for streams |
| 5.4 | Multi-platform | Simultaneous streams to multiple destinations |
| 5.5 | Stream Dashboard | Live viewer count, chat, health monitoring |

---
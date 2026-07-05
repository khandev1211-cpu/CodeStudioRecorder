# CodeStudio Recorder 🎥

**CodeStudio Recorder** is a high-performance, professional-grade screen recording platform for Windows. Built with a modern **Flutter** frontend and a specialized **C++20 Native Engine**, it is designed for creators, developers, and educators who need low-latency captures with real-time AI-driven effects.

![Build Status](https://github.com/khandev1211-cpu/CodeStudioRecorder/actions/workflows/release.yml/badge.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)

---

## 🚀 Project Status: Phase 5 (Live Production Ready)
The project has achieved **Production Stability** with a focus on real-time visual feedback and reliable hardware encoding.

- [x] **Native Transparent Overlay** (Live visual feedback for drawings/highlights)
- [x] **Stable H.264 Pipeline** (Even-dimension correction & auto-directory creation)
- [x] **Non-Blocking Finalization** (Background thread encoding to prevent UI hangs)
- [x] **System Integrity Suite** (Pre-flight requirement & permission diagnostics)
- [x] **Real-time Cursor Highlighting** (Direct2D Focus)
- [x] **Expanding Click Animations** (Global Hook Integration)
- [x] **Smart Zoom** (Smoothed follow-cam following cursor)
- [x] **Webcam PiP** (Hardware-accelerated Media Foundation feed)
- [x] **Hardware Encoding** (Auto-detection & fallback for NVENC, AMF, QSV)
- [x] **AI Foundation** (Noise gate, Silence detection, and Caption streaming bridge)

---

## ✨ Key Features

### 🖼️ Real-Time Presentation HUD
Unlike standard recorders, CodeStudio creates a native transparent overlay. You can see your annotations, cursor highlights, and zoom effects **live** on your screen as you record them, ensuring perfect delivery every time.

### 🤖 AI-Augmented Pipeline
- **Auto-Captions**: Real-time speech-to-text architecture with UI streaming support.
- **Silence Detection**: Energy-based VAD to automatically detect "dead air".
- **Noise Suppression**: Soft-gate filtering to clean microphone input in real-time.

### 🔥 Pro-Grade Performance
- **Zero-Copy Capturing**: Direct integration with Windows Graphics Capture.
- **Texture Pool**: Pre-allocated GPU buffers for zero-latency frame processing.
- **Multi-Device Audio**: Mix system loopback and professional XLR microphones via WASAPI with real-time level visualizers.

---

## 🏗 System Architecture

1.  **Flutter Layer**: Material 3 UI with **Riverpod** state management.
2.  **FFI Bridge**: High-speed binary communication between Dart and C++.
3.  **Native Engine (C++20)**: 
    *   `OverlayManager`: Handles the top-most transparent drawing canvas.
    *   `FFmpegEncoder`: Orchestrates hardware streams and MP4 containerization.
    *   `TexturePool`: Manages writable GPU buffers for real-time effects.

---

## 🚀 Getting Started (Users)

### Installation
1. **Download**: Get the latest `CodeStudioRecorder_Setup.exe` from [Releases](https://github.com/khandev1211-cpu/CodeStudioRecorder/releases).
2. **Setup**: The installer handles FFmpeg dependencies, license agreements, and system paths.
3. **Launch**: The app verifies DirectX 11 and Storage permissions on first run.

> **Note on Custom Logo**: To use your own branded icon for the application executable and taskbar, replace the file `windows/runner/resources/app_icon.ico` with your own `.ico` file before building or installing.

---

## 🗺 Roadmap

- **Phase 1-3**: Core, Creator Tools, and Extensibility. (COMPLETE)
- **Phase 4**: AI Foundations (Noise, Silence, Captions). (STABLE)
- **Phase 5**: Live Presentation & Performance. (STABLE)
- **Phase 6**: Local AI Inference (Whisper/RNNoise ONNX integration) & In-App Trimming. (UPCOMING)

---

Built with ❤️ by [Khandev](https://github.com/khandev1211-cpu)

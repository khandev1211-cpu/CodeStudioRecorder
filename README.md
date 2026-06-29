# CodeStudio Recorder 🎥

**CodeStudio Recorder** is a high-performance, professional-grade screen recording platform for Windows. Built with a modern **Flutter** frontend and a specialized **C++20 Native Engine**, it is designed for creators, developers, and educators who need low-latency, high-fidelity captures with real-time visual effects.

![Build Status](https://github.com/khandev1211-cpu/CodeStudioRecorder/actions/workflows/release.yml/badge.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)
![Flutter](https://img.shields.io/badge/Flutter-3.x-02569B?logo=flutter)
![C++](https://img.shields.io/badge/C++-20-00599C?logo=c%2B%2B)

---

## 🚀 Project Status: Phase 2 (Creator Tools)
The project is currently in **Phase 2: Professional Creator Tools**. We have successfully stabilized the hardware capture pipeline and are actively implementing real-time frame processors.

- [x] **Real-time Cursor Highlighting** (Direct2D)
- [x] **Expanding Click Animations** (Direct2D)
- [x] **Automated Cloud Builds** (GitHub Actions)
- [ ] **Smart Zoom** (In progress)

---

## ✨ Key Features

### 🔥 Pro-Grade Performance
- **WGC Capture**: Native integration with the Windows Graphics Capture API for 60FPS+ window and monitor recording.
- **Hardware Accelerated**: Optimized paths for **NVENC** (NVIDIA), **QuickSync** (Intel), and **AMF** (AMD).
- **Pro Audio**: Low-latency system loopback and microphone capture via **WASAPI**.
- **Zero-Copy Pipeline**: Ultra-efficient frame transfer using lock-free SPSC ring buffers and pre-allocated GPU texture pools.

### 🛠 Professional Effects (Real-time)
- **Cursor Focus**: Draw high-visibility highlights around the mouse cursor automatically.
- **Visual Clicks**: Dynamic ripple animations appear exactly where you click in the recording.
- **Permission Safe**: Smart path management handles Windows Security folder blocks automatically.

---

## 🏗 System Architecture

CodeStudio uses a strict **Layered Architecture** to ensure UI responsiveness and engine stability:

1.  **Flutter UI Layer**: A reactive Material 3 interface managed by **Riverpod**.
2.  **FFI Bridge**: A type-safe Dart-to-C++ bridge using **dart:ffi**.
3.  **Native Engine (C++20)**:
    *   **CaptureEngine**: WinRT-based frame acquisition.
    *   **AudioEngine**: WASAPI session management and mixing.
    *   **ProcessorPipeline**: Direct2D-powered real-time frame effects.
    *   **EncodingEngine**: FFmpeg-based hardware stream orchestration.

---

## 🚀 Getting Started

### Prerequisites
- **Flutter SDK** (^3.4.0)
- **Visual Studio 2022** (with "Desktop development with C++" workload)
- **Windows 10 1903+** (Required for WGC support)

### Quick Start
1. **Download**: Grab the latest `.zip` from the [Releases](https://github.com/khandev1211-cpu/CodeStudioRecorder/releases) page.
2. **Run**: Extract and launch `codestudio_recorder.exe`.

### Development Build
```bash
git clone https://github.com/khandev1211-cpu/CodeStudioRecorder.git
cd CodeStudioRecorder
flutter pub get
flutter run -d windows
```

---

## 🗺 Roadmap

- **Phase 1 (MVP)**: Core capture coordination, WASAPI audio, and history management. (STABLE)
- **Phase 2 (Creator Tools)**: Cursor effects, click animations, zoom-on-cursor. (ACTIVE)
- **Phase 3 (Plugins)**: C++ SDK for custom overlays and encoders.
- **Phase 4 (AI Features)**: Local Whisper captions and Smart Zoom.

---

Built with ❤️ by [Khandev](https://github.com/khandev1211-cpu)

# CodeStudio Recorder 🎥

**CodeStudio Recorder** is a high-performance, professional-grade screen recording platform for Windows. Built with a modern **Flutter** frontend and a specialized **C++20 Native Engine**, it is designed for creators, developers, and educators who need low-latency, high-fidelity captures with real-time visual and AI-driven effects.

![Build Status](https://github.com/khandev1211-cpu/CodeStudioRecorder/actions/workflows/release.yml/badge.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)
![Flutter](https://img.shields.io/badge/Flutter-3.x-02569B?logo=flutter)
![C++](https://img.shields.io/badge/C++-20-00599C?logo=c%2B%2B)

---

## 🚀 Project Status: Phase 4 (AI Augmented)
The project has evolved into **Phase 4: AI Augmented Production**. We have successfully integrated local AI models for audio and video enhancement, alongside a robust plugin architecture.

- [x] **Real-time Cursor Highlighting** (Direct2D)
- [x] **Expanding Click Animations** (Direct2D + Global Hook)
- [x] **Smart Zoom** (Smoothed cursor tracking)
- [x] **Webcam PiP** (Media Foundation integration)
- [x] **Audio Visualization** (Real-time peak meters)
- [x] **Hardware-specific Encoding** (NVENC, AMF, QSV)
- [x] **C++ Plugin SDK** (Extensible frame processors)
- [x] **AI Noise Removal** (Real-time audio cleaning)
- [x] **AI Auto-Captions** (Local Whisper integration)
- [x] **AI Silence Detection** (Energy-based VAD)
- [x] **System Integrity** (Pre-flight requirement checks)

---

## ✨ Key Features

### 🔥 Pro-Grade Performance
- **WGC Capture**: Native integration with the Windows Graphics Capture API for 60FPS+ capture.
- **Hardware Accelerated**: Optimized paths for **NVENC** (NVIDIA), **QuickSync** (Intel), and **AMF** (AMD).
- **Zero-Copy Pipeline**: Ultra-efficient frame transfer using GPU texture pools.
- **Pro Audio**: Multi-device support via **WASAPI** with real-time level monitoring.

### 🤖 AI-Native Pipeline
- **Local Whisper Captions**: Real-time speech-to-text generated locally on your machine.
- **Smart Zoom**: Professional follow-cam logic that smoothly tracks your workspace.
- **Noise Suppression**: AI-driven audio cleaning to remove fans and background hum.

### 🧩 Extensibility & Tools
- **Plugin SDK**: Build custom overlays and effects in C++ and load them as DLLs.
- **Profile Management**: Save custom configurations for different recording scenarios.
- **Annotation Suite**: Draw directly on the screen during recording (lines, arrows, shapes).

---

## 🏗 System Architecture

CodeStudio uses a strict **Layered Architecture** to ensure UI responsiveness and engine stability:

1.  **Flutter UI Layer**: A reactive Material 3 interface managed by **Riverpod**.
2.  **FFI Bridge**: A high-performance type-safe bridge using **dart:ffi**.
3.  **Native Engine (C++20)**:
    *   **CaptureEngine**: WinRT-based frame acquisition.
    *   **AudioEngine**: WASAPI session management and mixing.
    *   **AIProcessor**: Local inference for VAD, Noise, and Captions.
    *   **PluginManager**: Dynamic DLL loader for external processors.
    *   **EncodingEngine**: FFmpeg-based hardware stream orchestration.

---

## 🚀 Getting Started

### Installation
1. **Download**: Grab the latest `CodeStudioRecorder_Setup.exe` from the [Releases](https://github.com/khandev1211-cpu/CodeStudioRecorder/releases) page.
2. **Install**: Run the installer. It will handle system checks and configure the environment.
3. **Launch**: Open from your Start Menu. The app will perform a quick integrity check on first run.

### Prerequisites (Development)
- **Flutter SDK** (^3.4.0)
- **Visual Studio 2022** (with "Desktop development with C++" workload)
- **Windows 10 1903+** (Required for WGC support)

### Development Build
```bash
git clone https://github.com/khandev1211-cpu/CodeStudioRecorder.git
cd CodeStudioRecorder
flutter pub get
flutter run -d windows
```

---

## 📚 Documentation

### 1. Project Structure
- **`lib/`**: Flutter frontend implementation (Feature-first architecture).
- **`windows/native_engine/`**: High-performance C++20 backend.
    - `src/`: Core implementation (Capture, Audio, AI, Encoding).
    - `include/`: FFI headers and Plugin SDK interfaces.
- **`docs/`**: Detailed technical guides for each module.

### 2. Building from Source
#### Native Engine (C++)
The engine is built automatically by Flutter, but can be built manually via CMake:
```bash
cd windows
cmake -B build
cmake --build build --config Release
```

---

## 🗺 Roadmap

- **Phase 1 (MVP)**: Core capture and history management. (STABLE)
- **Phase 2 (Creator Tools)**: Cursor effects and Smart Zoom. (STABLE)
- **Phase 3 (Extensibility)**: C++ Plugin SDK and Webcam support. (STABLE)
- **Phase 4 (AI Features)**: Local Whisper captions and Noise removal. (STABLE)
- **Phase 5 (Post-Production)**: In-app editor and AI-driven highlight clipping. (UPCOMING)

---

Built with ❤️ by [Khandev](https://github.com/khandev1211-cpu)

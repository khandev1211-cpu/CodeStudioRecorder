# CodeStudio Recorder 🎥

**CodeStudio Recorder** is a high-performance, professional-grade screen recording platform for Windows. Built with a **Flutter** frontend and a specialized **C++ Native Engine**, it is designed for creators, developers, and educators who need low-latency, high-fidelity captures with AI-powered features.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)
![Flutter](https://img.shields.io/badge/Flutter-3.x-02569B?logo=flutter)
![C++](https://img.shields.io/badge/C++-17-00599C?logo=c%2B%2B)

---

## ✨ Key Features

### 🚀 Performance & Capture
- **Hardware Accelerated**: Leverages NVENC (NVIDIA), QSV (Intel), and AMF (AMD) for zero-lag encoding.
- **WGC Capture**: Uses modern Windows Graphics Capture API for high-frame-rate window and monitor recording.
- **Audio Loopback**: High-quality system audio and microphone capture via WASAPI.
- **Zero-Copy Pipeline**: Optimized frame transfer using lock-free ring buffers and texture pooling.

### 🛠 Creator Tools (Phase 2+)
- **Smart Zoom**: AI-powered cursor tracking that automatically zooms into active areas.
- **Cursor Effects**: Highlighting, click animations, and ripple effects.
- **Auto-Captions**: Local Whisper-based transcription for real-time burned-in subtitles.

### 🔌 Extensibility
- **Plugin SDK**: A robust C++ SDK for building custom overlays, encoders, and post-processors.
- **Sandboxed Execution**: Plugins run in isolated processes for maximum stability.

---

## 🏗 Architecture Overview

CodeStudio follows a strict **Layered Architecture**:

1.  **Flutter UI Layer**: A reactive, modern interface built with Riverpod and GoRouter.
2.  **FFI Bridge**: A high-performance Dart FFI layer for type-safe communication with the engine.
3.  **Native Engine (C++)**: The performance core.
    *   **CaptureEngine**: Manages D3D11 surfaces and WGC sessions.
    *   **AudioEngine**: Handles WASAPI capture and mixing.
    *   **EncodingEngine**: Coordinates hardware encoders and FFmpeg muxers.

---

## 🚀 Getting Started

### Prerequisites
- **Flutter SDK** (^3.11.5)
- **Visual Studio 2022** (with "Desktop development with C++" workload)
- **CMake** (3.10+)
- **Windows 10 1903+** (Required for WGC)
- **FFmpeg 6.0+ Development Libraries**:
    - Download the "shared" or "dev" build from [gyan.dev](https://www.gyan.dev/ffmpeg/builds/).
    - Extract to `third_party/ffmpeg/`.
    - Ensure headers are in `third_party/ffmpeg/include` and `.lib` files are in `third_party/ffmpeg/lib/x64`.
    - Place the `.dll` files in your system path or copy them to the build output folder.

### Installation & Build

1.  **Clone the repository**:
    ```bash
    git clone https://github.com/khandev1211-cpu/CodeStudioRecorder.git
    cd CodeStudioRecorder
    ```

2.  **Get Dart dependencies**:
    ```bash
    flutter pub get
    ```

3.  **Run the application**:
    ```bash
    flutter run -d windows
    ```

---

## 🗺 Roadmap

- **Phase 1 (MVP)**: Core recording coordination, WGC capture, WASAPI audio, and basic history/settings. (IN PROGRESS)
- **Phase 2 (Professional Tools)**: Cursor effects, zoom-on-cursor, and export presets.
- **Phase 3 (Plugins)**: SDK release and plugin host implementation.
- **Phase 4 (AI)**: Local Whisper captions, smart zoom, and noise removal.
- **Phase 5 (Streaming)**: RTMP support and multi-scene management.

See [Roadmap.md](docs/12_Roadmap.md) for detailed milestones.

---

## 🤝 Contributing

Contributions are welcome! Please read our [Contributing Guidelines](docs/01_Project_Overview.md) before submitting pull requests.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

Built with ❤️ by [Khandev](https://github.com/khandev1211-cpu)

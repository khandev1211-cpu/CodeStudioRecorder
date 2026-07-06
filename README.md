# CodeStudio Recorder

> **A high-performance, professional-grade screen recording platform for Windows** — built with Flutter and a specialized C++20 Native Engine. Designed for developers, educators, and content creators who need low-latency captures with real-time AI-driven effects.

<p align="center">
  <img src="https://img.shields.io/github/actions/workflow/status/khandev1211-cpu/CodeStudioRecorder/release.yml?branch=main&label=Build&logo=github" alt="Build Status">
  <img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="License">
  <img src="https://img.shields.io/badge/platform-Windows_10%2B-blue?logo=windows" alt="Platform">
  <img src="https://img.shields.io/badge/flutter-3.19%2B-02569B?logo=flutter" alt="Flutter">
  <img src="https://img.shields.io/badge/c%2B%2B-20-00599C?logo=c%2B%2B" alt="C++20">
  <img src="https://img.shields.io/badge/phase-5_%E2%80%93_Production_Ready-success" alt="Phase 5">
</p>

---

## 📸 Screenshots

<table>
  <tr>
    <td align="center" width="50%">
      <img src="App Screenshots/Costudio Main Page.png" alt="CodeStudio Main Page" width="100%">
      <br />
      <sub><b>Main Dashboard</b> — Clean, distraction-free UI built with Material 3</sub>
    </td>
    <td align="center" width="50%">
      <img src="App Screenshots/recordingWithtools.png" alt="Recording with Tools" width="100%">
      <br />
      <sub><b>Recording Mode</b> — Real-time overlay with annotation tools, cursor highlights, and zoom effects</sub>
    </td>
  </tr>
  <tr>
    <td align="center" width="50%" colspan="2">
      <img src="App Screenshots/SettingPage.png" alt="Settings Page" width="80%">
      <br />
      <sub><b>Settings Panel</b> — Comprehensive configuration for audio, video, hotkeys, and AI features</sub>
    </td>
  </tr>
</table>

---

## ✨ Key Features

### 🖼️ Real-Time Presentation HUD
Unlike standard recorders, CodeStudio creates a **native transparent overlay** that lets you see your annotations, cursor highlights, and zoom effects **live** on your screen as you record — ensuring perfect delivery every time.

| Feature | Status |
|---------|--------|
| Native Transparent Overlay | ✅ **Live** |
| Real-time Cursor Highlighting (Direct2D Focus) | ✅ **Live** |
| Expanding Click Animations (Global Hook Integration) | ✅ **Live** |
| Smart Zoom (Smoothed follow-cam) | ✅ **Live** |
| Webcam PiP (Hardware-accelerated Media Foundation) | ✅ **Live** |

### 🎬 Professional Recording Engine
- **Zero-Copy Capturing** — Direct integration with Windows Graphics Capture API (DXGI / D3D)
- **Hardware Encoding** — Auto-detection & fallback for **NVENC**, **AMD AMF**, and **Intel Quick Sync**
- **Stable H.264 Pipeline** — Even-dimension correction & auto-directory creation
- **Non-Blocking Finalization** — Background thread encoding prevents UI hangs
- **Texture Pool** — Pre-allocated GPU buffers for zero-latency frame processing

### 🎙️ Multi-Device Audio
Mix **system loopback** and professional **XLR microphones** via WASAPI with real-time level visualizers. Supports:
- System audio capture
- Microphone capture
- Real-time audio mixing
- Multi-device input

### 🤖 AI-Augmented Pipeline
| AI Feature | Status | Description |
|-----------|--------|-------------|
| **Auto-Captions** | 🛠️ Architecture Ready | Real-time speech-to-text with UI streaming support |
| **Silence Detection** | ✅ **Live** | Energy-based VAD to automatically detect "dead air" |
| **Noise Suppression** | ✅ **Live** | Soft-gate filtering to clean microphone input in real-time |
| **Smart Zoom** | ✅ **Live** | AI cursor tracking with smoothed follow-cam |
| **Highlight Generation** | 🔮 Upcoming | LLM + activity heuristics for auto highlight reels |
| **Background Noise Removal** | 🔮 Upcoming | RNNoise / DeepFilterNet integration |
| **Auto Chapter Markers** | 🔮 Upcoming | LLM + scene detection model |

### 🛡️ System Integrity
- **System Integrity Suite** — Pre-flight requirement & permission diagnostics
- **Auto-directory creation** for organized output management
- **Error recovery** with detailed diagnostics

---

## 🏗️ Architecture

CodeStudio Recorder is built on a **strict layered architecture** with clear separation between the Flutter UI layer, the Application Controller, the Native Engine, and the Export Pipeline.

```
┌─────────────────────────────────────────────┐
│              Flutter UI Layer               │
│  (Dart — Material 3, Riverpod, Widgets)     │
├─────────────────────────────────────────────┤
│              FFI / Platform Channel         │
├─────────────────────────────────────────────┤
│           Application Controller            │
│   (C++ — Session, Config, Lifecycle Mgmt)   │
├──────────────────┬──────────────────────────┤
│  Recording Engine│  Database (SQLite)       │
├──────────────────┴──────────────────────────┤
│              Capture Engine                 │
│  (Windows Graphics Capture, DXGI, Direct3D) │
├─────────────────────────────────────────────┤
│            Encoding Engine                  │
│  (FFmpeg — NVENC, AMF, QSV, H.264/AVC)     │
├─────────────────────────────────────────────┤
│         Audio Engine (WASAPI)               │
│  (Loopback, Mic input, Mixing, VAD)         │
└─────────────────────────────────────────────┘
```

### Tech Stack

| Layer | Technology |
|-------|-----------|
| **UI Framework** | Flutter 3.19+ (Dart) with Material 3 Design |
| **State Management** | Riverpod |
| **Native Engine** | C++20 |
| **Screen Capture** | Windows Graphics Capture API (DXGI) |
| **Video Encoding** | FFmpeg (NVENC, AMF, QSV hardware acceleration) |
| **Audio** | WASAPI (Windows Audio Session API) |
| **AI/ML** | ONNX Runtime, Whisper, RNNoise |
| **Database** | SQLite |
| **Build System** | CMake (C++) + Flutter toolchain (Dart) |

---

## 🚀 Getting Started

### Prerequisites
- **Windows 10 or later** (64-bit)
- **DirectX 11** compatible GPU (for hardware acceleration)
- **4GB RAM** minimum (8GB+ recommended)

### Installation
1. **Download** the latest `CodeStudioRecorder_Setup.exe` from the [Releases Page](https://github.com/khandev1211-cpu/CodeStudioRecorder/releases)
2. **Run the installer** — it handles FFmpeg dependencies, license agreements, and system paths automatically
3. **Launch** — the app verifies DirectX 11 and storage permissions on first run

### Building from Source

```powershell
# Clone the repository
git clone https://github.com/khandev1211-cpu/CodeStudioRecorder.git
cd CodeStudioRecorder

# Build the native engine (C++)
cd windows/native_engine
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Build the Flutter app
cd ../..
flutter pub get
flutter build windows --release

# Build installer (optional)
iscc installer.iss
```

See the full [Build & Deployment Guide](docs/10_Build_Deployment.md) for detailed instructions.

### Custom Branding
To use your own branded icon for the application executable and taskbar, replace `windows/runner/resources/app_icon.ico` with your own `.ico` file before building.

---

## 🗺️ Roadmap

| Phase | Focus | Status |
|-------|-------|--------|
| **Phase 1** | Core Recording Engine (Capture, Audio, Basic Export) | ✅ **Complete** |
| **Phase 2** | Creator Tools (Annotations, Zoom, Cursor Effects) | ✅ **Complete** |
| **Phase 3** | Plugin SDK & Extensibility | ✅ **Complete** |
| **Phase 4** | AI Foundations (Noise Gate, VAD, Caption Bridge) | ✅ **Stable** |
| **Phase 5** | Live Presentation & Performance Optimization | ✅ **Stable** |
| **Phase 6** | Local AI Inference (Whisper/RNNoise ONNX) & In-App Trimming | 🔄 **Upcoming** |
| **Phase 7** | Streaming (RTMP, Multi-platform, Scene Management) | 🔮 **Future** |

---

## 📚 Documentation

Comprehensive documentation is available in the [`docs/`](docs/) directory:

| Document | Description |
|----------|-------------|
| [Project Overview](docs/01_Project_Overview.md) | Vision, mission, target audience & competitor analysis |
| [System Architecture](docs/02_System_Architecture.md) | Complete architecture, layers, threading & IPC |
| [Recording Engine](docs/03_Recording_Engine.md) | Frame pipeline, lifecycle & synchronization |
| [Audio Engine](docs/04_Audio_Engine.md) | WASAPI integration, mixing & VAD |
| [Encoding Engine](docs/05_Encoding_Engine.md) | FFmpeg pipeline, hardware encoding & containers |
| [AI Architecture](docs/06_AI_Architecture.md) | AI/ML pipeline, models & roadmap |
| [Database](docs/07_Database.md) | SQLite schema, migrations & queries |
| [Plugin SDK](docs/08_Plugin_SDK.md) | Plugin system, API & extension points |
| [Flutter UI](docs/09_Flutter_UI.md) | Widget tree, state management & theming |
| [Build & Deployment](docs/10_Build_Deployment.md) | Build system, CI/CD & packaging |
| [Testing & CI/CD](docs/11_Testing_CI_CD.md) | Testing strategy, automation & pipelines |
| [API Reference](docs/13_API_Reference.md) | Complete API documentation |
| [Security](docs/14_Security.md) | Security model, permissions & best practices |
| [Performance Optimization](docs/15_Performance_Optimization.md) | Profiling, bottlenecks & tuning |
| [Coding Standards](docs/16_Coding_Standards.md) | Style guides, conventions & linting |
| [Contribution Guide](docs/19_Contribution_Guide.md) | How to contribute, PR workflow & code review |

---

## 🤝 Contributing

We welcome contributions! Please see our [Contribution Guide](docs/19_Contribution_Guide.md) for details on:

- Code of Conduct
- Development setup
- Pull request process
- Coding standards
- Testing requirements

---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

<p align="center">
  Built with ❤️ by <a href="https://github.com/khandev1211-cpu">Khandev</a>
  <br />
  <sub>CodeStudio Recorder — Professional Screen Recording for Windows</sub>
</p>
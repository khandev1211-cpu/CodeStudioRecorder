# 10 — Build & Deployment

## CodeStudio Recorder — Build System & Deployment

---

## Table of Contents
1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Repository Structure](#repository-structure)
4. [Build System](#build-system)
5. [Native Engine Build (C++)](#native-engine-build)
6. [Flutter Build](#flutter-build)
7. [Release Process](#release-process)
8. [Installers & Packaging](#installers--packaging)
9. [Dependencies](#dependencies)
10. [Versioning](#versioning)

---

## Overview

CodeStudio Recorder uses a two-part build system:
- **CMake** for the native C++ engine (`codestudio_engine.dll`)
- **Flutter** for the Dart/Flutter UI application

Both are orchestrated by PowerShell build scripts for local development and GitHub Actions for CI/CD.

---

## Prerequisites

| Tool | Version | Purpose |
|---|---|---|
| Visual Studio 2022 | 17.x | C++ compiler, Windows SDK |
| CMake | 3.26+ | Native engine build system |
| Flutter SDK | 3.19+ | UI build |
| Dart SDK | 3.3+ | Bundled with Flutter |
| NVIDIA CUDA Toolkit | 12.x | NVENC encoder SDK |
| Intel oneVPL SDK | 2.x | Quick Sync encoder SDK |
| AMD AMF SDK | 1.4.x | AMF encoder SDK |
| NSIS | 3.x | Installer packaging |
| Git | 2.40+ | Source control |
| Python | 3.11+ | Build scripts |

---

## Build System

### Top-Level Build Script

```powershell
# scripts/build.ps1

param(
    [string]$Config = "Release",   # Debug | Release | RelWithDebInfo
    [switch]$NativeOnly,
    [switch]$UIOnly,
    [switch]$Package
)

if (-not $UIOnly) {
    Write-Host "Building native engine..."
    & cmake -B build/native -S engine -DCMAKE_BUILD_TYPE=$Config
    & cmake --build build/native --config $Config --parallel
}

if (-not $NativeOnly) {
    Write-Host "Building Flutter UI..."
    Copy-Item "build/native/$Config/codestudio_engine.dll" "windows/runner/"
    & flutter build windows --$Config.ToLower()
}

if ($Package) {
    Write-Host "Packaging installer..."
    & python scripts/make_installer.py --config $Config
}
```

---

## Native Engine Build (C++)

### CMakeLists.txt (top-level)

```cmake
cmake_minimum_required(VERSION 3.26)
project(CodeStudioEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(FFmpeg REQUIRED COMPONENTS avcodec avformat avutil swresample)
find_package(SQLite3 REQUIRED)

# Subdirectories
add_subdirectory(capture)
add_subdirectory(audio)
add_subdirectory(encoder)
add_subdirectory(database)
add_subdirectory(plugins)

# Main engine DLL
add_library(codestudio_engine SHARED
    src/engine.cpp
    src/session_manager.cpp
    src/recording_engine.cpp
    src/config_manager.cpp
    src/ffi_exports.cpp
)

target_link_libraries(codestudio_engine
    PRIVATE
        cs_capture
        cs_audio
        cs_encoder
        cs_database
        cs_plugins
        FFmpeg::avcodec
        FFmpeg::avformat
        FFmpeg::avutil
        FFmpeg::swresample
        SQLite3::SQLite3
        d3d11.lib
        dxgi.lib
        mf.lib
        mfuuid.lib
        ole32.lib
        windowscodecs.lib
)

# Export header
target_include_directories(codestudio_engine PUBLIC include/)
```

### Build Configurations

| Config | Optimizations | Logging | Debug Symbols |
|---|---|---|---|
| Debug | None | Verbose | Full |
| RelWithDebInfo | O2 | Info | Full |
| Release | O3, LTO | Warning only | None |

---

## Flutter Build

```bash
# Development
flutter run -d windows

# Release build
flutter build windows --release

# Build output location:
# build/windows/x64/runner/Release/
```

### Asset Bundling

```yaml
# pubspec.yaml
flutter:
  assets:
    - assets/icons/
    - assets/sounds/
    - assets/models/         ← AI model files
  
  fonts:
    - family: JetBrainsMono
      fonts:
        - asset: assets/fonts/JetBrainsMono-Regular.ttf
        - asset: assets/fonts/JetBrainsMono-Bold.ttf
```

---

## Release Process

```
1. Version bump (scripts/bump_version.ps1)
   → Updates CMakeLists.txt version
   → Updates pubspec.yaml version
   → Creates git tag vX.Y.Z

2. CI builds triggered (GitHub Actions)
   → Native engine: cmake build
   → Flutter: flutter build windows
   → Tests: ctest + flutter test
   → Sign artifacts

3. Package
   → Copy engine DLL to Flutter output
   → NSIS installer generated
   → Portable ZIP generated

4. Release
   → Draft GitHub Release
   → Upload installer + portable ZIP
   → Publish release notes
```

---

## Installers & Packaging

### NSIS Installer

```nsi
; installer.nsi

Name "CodeStudio Recorder"
OutFile "CodeStudioRecorder-Setup-${VERSION}.exe"
InstallDir "$PROGRAMFILES64\CodeStudio Recorder"

Section "Install"
  SetOutPath "$INSTDIR"
  File /r "build\release\*.*"
  
  ; Create shortcuts
  CreateShortcut "$DESKTOP\CodeStudio Recorder.lnk" "$INSTDIR\codestudio.exe"
  CreateShortcut "$SMPROGRAMS\CodeStudio Recorder\CodeStudio Recorder.lnk" "$INSTDIR\codestudio.exe"
  
  ; Register uninstaller
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CodeStudioRecorder" \
    "DisplayName" "CodeStudio Recorder"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CodeStudioRecorder" \
    "UninstallString" "$INSTDIR\uninstall.exe"
  
  WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd
```

### Portable ZIP

```
CodeStudioRecorder-${VERSION}-portable.zip
├── codestudio.exe
├── codestudio_engine.dll
├── data/
│   └── (ffmpeg DLLs, model files, etc.)
└── README.txt
```

---

## Dependencies

### Bundled (shipped with installer)

| Library | Version | License |
|---|---|---|
| FFmpeg | 6.1 | LGPL 2.1 |
| SQLite | 3.45 | Public Domain |
| ONNX Runtime | 1.17 | MIT |
| RNNoise | — | BSD |

### System Requirements

| Component | Minimum |
|---|---|
| Windows | Windows 10 20H1 (2004) or later |
| RAM | 4GB |
| GPU | DirectX 11 compatible |
| Storage | 200MB for install |

---

## Versioning

Semantic versioning: `MAJOR.MINOR.PATCH[-prerelease]`

```
1.0.0         → First stable release
1.0.1         → Patch (bug fix)
1.1.0         → Minor (new feature, backward compatible)
2.0.0         → Major (breaking change)
1.1.0-beta.1  → Pre-release
```

Version string embedded in binary:
```cpp
#define CS_VERSION_MAJOR 1
#define CS_VERSION_MINOR 0
#define CS_VERSION_PATCH 0
#define CS_VERSION_STRING "1.0.0"
#define CS_BUILD_DATE     __DATE__
#define CS_BUILD_COMMIT   "abc1234"   // injected by CI
```

---

*Last updated: 2025 | Module 10 of 19*

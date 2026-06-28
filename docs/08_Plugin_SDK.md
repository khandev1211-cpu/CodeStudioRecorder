# 08 — Plugin SDK

## CodeStudio Recorder — Plugin SDK

---

## Table of Contents

1. [Overview](#overview)
2. [Plugin Lifecycle](#plugin-lifecycle)
3. [Extension Points](#extension-points)
4. [Plugin SDK APIs](#plugin-sdk-apis)
5. [Permissions Model](#permissions-model)
6. [SDK Architecture](#sdk-architecture)
7. [Versioning and Compatibility](#versioning-and-compatibility)
8. [Writing Your First Plugin](#writing-your-first-plugin)
9. [Plugin Manifest](#plugin-manifest)
10. [Distribution](#distribution)

---

## Overview

The CodeStudio Recorder Plugin SDK enables third-party developers to extend the recorder with custom functionality — new export formats, custom overlays, integrations with external tools, AI processing stages, and more.

Plugins run inside a **sandboxed host process** and interact with the engine only through the published SDK interface. They cannot directly access engine internals, OS resources beyond their declared permissions, or other plugins' data.

---

## Plugin Lifecycle

```
[Install]
    │ plugin manifest validated
    │ permissions reviewed by user
    ▼
[Loaded]
    │ plugin DLL loaded into PluginHost process
    │ Plugin::onLoad() called
    ▼
[Active]
    │ Plugin registers its extension points
    │ Plugin::onActivate() called
    ▼
[Recording Start] → Plugin::onRecordingStart(RecordingContext)
    │
[Frame Event]     → Plugin::onFrame(FrameContext) [if subscribed]
    │
[Audio Event]     → Plugin::onAudio(AudioContext) [if subscribed]
    │
[Recording Stop]  → Plugin::onRecordingStop(RecordingResult)
    │
[Export Event]    → Plugin::onExport(ExportContext) [if subscribed]
    │
[Deactivate]      → Plugin::onDeactivate()
    │
[Unload]          → Plugin::onUnload()
```

---

## Extension Points

Plugins can extend the following areas:

| Extension Point | Description | Example Use Case |
|---|---|---|
| `IFrameProcessor` | Process video frames in real-time | Custom overlay, blur face, watermark |
| `IAudioProcessor` | Process audio buffers in real-time | Custom EQ, VST bridge |
| `IExportFormat` | Add new export format | APNG exporter, custom encoder |
| `IOverlay` | Render HUD overlay on recording | Keystroke display, FPS counter |
| `ISettingsPage` | Add settings UI page | Plugin configuration panel |
| `IToolbarAction` | Add toolbar button | Quick action trigger |
| `IPostProcessor` | Post-process completed recordings | Upload to S3, trim silence |
| `IHighlightDetector` | Custom highlight scoring | Game-specific events |

---

## Plugin SDK APIs

### Core SDK Header

```cpp
// codestudio_sdk.h

#pragma once
#include "cs_types.h"
#include "cs_frame.h"
#include "cs_audio.h"
#include "cs_recording.h"
#include "cs_ui.h"
#include "cs_storage.h"
#include "cs_logger.h"

namespace cs {

// Base plugin interface — all plugins implement this
class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    // Called once when the plugin DLL is loaded
    virtual PluginInfo getInfo() const = 0;
    
    // Lifecycle
    virtual bool onLoad(PluginHost* host) = 0;
    virtual void onUnload() = 0;
    virtual void onActivate() = 0;
    virtual void onDeactivate() = 0;
    
    // Recording events
    virtual void onRecordingStart(const RecordingContext& ctx) {}
    virtual void onRecordingStop(const RecordingResult& result) {}
    virtual void onRecordingPause() {}
    virtual void onRecordingResume() {}
};

// Exported entry point — every plugin DLL must export this
extern "C" __declspec(dllexport)
cs::IPlugin* cs_create_plugin();

} // namespace cs
```

---

### Frame Processing API

```cpp
class IFrameProcessor {
public:
    virtual ~IFrameProcessor() = default;
    
    // Called for every captured frame
    // frame is writable — modifications affect the recorded output
    virtual void processFrame(FrameContext& frame) = 0;
    
    // Priority: lower runs first (default: 100)
    virtual int priority() const { return 100; }
};
```

```cpp
// FrameContext exposes:
struct FrameContext {
    // Read-only metadata
    uint32_t width;
    uint32_t height;
    int64_t  timestampMs;
    uint32_t frameNumber;
    
    // Writable render target (Direct2D / Direct3D surface)
    ID2D1RenderTarget* d2dTarget;   // for 2D overlay drawing
    ID3D11Texture2D*   d3dTexture;  // for compute shader access
};
```

---

### Audio Processing API

```cpp
class IAudioProcessor {
public:
    virtual ~IAudioProcessor() = default;
    
    // Called for every audio buffer
    // buf.samples is writable
    virtual void processAudio(AudioContext& buf) = 0;
    
    virtual int priority() const { return 100; }
};

struct AudioContext {
    float*   samples;       // interleaved float32 PCM
    uint32_t frameCount;    // number of audio frames
    uint32_t channelCount;  // 1 = mono, 2 = stereo
    uint32_t sampleRate;    // e.g. 48000
    int64_t  timestampMs;
};
```

---

### Storage API

Plugins have an isolated key-value store backed by the `plugin_data` SQLite table:

```cpp
class IPluginStorage {
public:
    virtual bool        set(const std::string& key, const std::string& value) = 0;
    virtual std::string get(const std::string& key,
                            const std::string& defaultValue = "") = 0;
    virtual bool        remove(const std::string& key) = 0;
    virtual bool        has(const std::string& key) = 0;
    virtual std::vector<std::string> keys() = 0;
};
```

---

### Logger API

```cpp
class ILogger {
public:
    virtual void debug(const std::string& msg) = 0;
    virtual void info(const std::string& msg)  = 0;
    virtual void warn(const std::string& msg)  = 0;
    virtual void error(const std::string& msg) = 0;
};
```

All plugin log output is prefixed with the plugin ID and routed to the application log file.

---

### UI Registration API

```cpp
class IPluginHost {
public:
    // Register extension points
    virtual void registerFrameProcessor(IFrameProcessor* processor) = 0;
    virtual void registerAudioProcessor(IAudioProcessor* processor) = 0;
    virtual void registerExportFormat(IExportFormat* format) = 0;
    virtual void registerOverlay(IOverlay* overlay) = 0;
    virtual void registerSettingsPage(ISettingsPage* page) = 0;
    virtual void registerToolbarAction(IToolbarAction* action) = 0;
    
    // Accessors
    virtual IPluginStorage* storage() = 0;
    virtual ILogger*        logger() = 0;
    virtual RecordingConfig currentConfig() const = 0;
};
```

---

## Permissions Model

Plugins declare required permissions in their manifest. Users approve permissions on install.

| Permission | Description | Risk Level |
|---|---|---|
| `frame_read` | Read video frames | Low |
| `frame_write` | Modify video frames | Medium |
| `audio_read` | Read audio buffers | Low |
| `audio_write` | Modify audio buffers | Medium |
| `file_write` | Write files to disk | Medium |
| `network` | Make HTTP requests | High |
| `settings_read` | Read app settings | Low |
| `settings_write` | Modify app settings | High |
| `recording_control` | Start/stop recording | High |

---

## SDK Architecture

```
[Plugin DLL]
    │ implements IPlugin, IFrameProcessor, etc.
    │
[PluginHost Process]  ← sandboxed, isolated from main process
    │ PluginLoader: loads + validates DLL
    │ PluginRegistry: tracks registered extension points
    │ PermissionGuard: enforces declared permissions at runtime
    │
[IPC Pipe] ← Named pipe between PluginHost ↔ Engine
    │
[Application Controller]
    │ calls registered processors via stable ABI
    │
[Engine Subsystems]
```

### ABI Stability

The SDK uses a **COM-inspired interface approach** — all interfaces are pure abstract C++ classes with no data members. This ensures ABI stability across compiler versions and codegen differences.

All structs passed across the plugin boundary are **plain C structs** (no C++ objects, no vtables in data).

---

## Versioning and Compatibility

### SDK Version

```cpp
#define CS_SDK_VERSION_MAJOR 1
#define CS_SDK_VERSION_MINOR 0
#define CS_SDK_VERSION_PATCH 0
```

### Compatibility Rules

- **Major version change** → breaking API change; old plugins may not load
- **Minor version change** → new APIs added, existing APIs unchanged; backward compatible
- **Patch version change** → bug fixes only; fully compatible

On plugin load, the host checks:

```cpp
if (pluginInfo.sdkVersionMajor != CS_SDK_VERSION_MAJOR) {
    // Incompatible — refuse to load
    log.error("Plugin {} requires SDK v{}.x, host is v{}.x",
              pluginInfo.id, pluginInfo.sdkVersionMajor, CS_SDK_VERSION_MAJOR);
    return false;
}
```

---

## Writing Your First Plugin

### Minimal Plugin Example

```cpp
// my_watermark_plugin.cpp
#include "codestudio_sdk.h"
#include <d2d1.h>
#include <string>

class WatermarkPlugin : public cs::IPlugin, public cs::IFrameProcessor {
public:
    cs::PluginInfo getInfo() const override {
        return {
            .id      = "com.example.watermark",
            .name    = "Watermark Plugin",
            .version = "1.0.0",
            .author  = "Example Dev",
            .sdkVersionMajor = 1,
            .sdkVersionMinor = 0
        };
    }
    
    bool onLoad(cs::IPluginHost* host) override {
        host_ = host;
        text_ = host->storage()->get("watermark_text", "© My Channel");
        host->registerFrameProcessor(this);
        host->logger()->info("Watermark plugin loaded");
        return true;
    }
    
    void onUnload() override {}
    void onActivate() override {}
    void onDeactivate() override {}
    
    void processFrame(cs::FrameContext& frame) override {
        auto* rt = frame.d2dTarget;
        // Draw watermark text in bottom-right corner
        rt->BeginDraw();
        // ... D2D text drawing code ...
        rt->EndDraw();
    }

private:
    cs::IPluginHost* host_ = nullptr;
    std::string      text_;
};

extern "C" __declspec(dllexport)
cs::IPlugin* cs_create_plugin() {
    return new WatermarkPlugin();
}
```

---

## Plugin Manifest

Every plugin ships with a `plugin.json` manifest:

```json
{
  "id": "com.example.watermark",
  "name": "Watermark Plugin",
  "version": "1.0.0",
  "description": "Adds a customizable watermark overlay to recordings",
  "author": "Example Developer",
  "authorUrl": "https://example.com",
  "license": "MIT",
  
  "sdkVersion": "1.0",
  "minHostVersion": "1.0.0",
  
  "permissions": [
    "frame_write",
    "settings_read"
  ],
  
  "entry": "watermark_plugin.dll",
  
  "settingsSchema": {
    "watermark_text": {
      "type": "string",
      "default": "© My Channel",
      "label": "Watermark Text"
    },
    "watermark_opacity": {
      "type": "float",
      "default": 0.7,
      "min": 0.1,
      "max": 1.0,
      "label": "Opacity"
    }
  }
}
```

---

## Distribution

### Plugin Package Format

Plugins are distributed as `.csplugin` files — a ZIP archive with a specific structure:

```
my-plugin.csplugin
├── plugin.json          ← manifest
├── watermark_plugin.dll ← compiled plugin
├── assets/              ← optional resources
│   └── icon.png
└── README.md            ← optional documentation
```

### Installation

Users install plugins via:
1. **Plugin Manager UI** — drag-and-drop or browse
2. **Command line:** `codestudio install-plugin my-plugin.csplugin`
3. **Plugin marketplace** (Phase 3) — one-click install

---

*Last updated: 2025 | Module 08 of 19*

# 14 — Security

## CodeStudio Recorder — Security, Privacy & Threat Model

---

## Overview

CodeStudio Recorder handles sensitive data — screen content, audio, and user settings. This document defines the security model, coding practices, privacy guarantees, and threat mitigations.

---

## Privacy Guarantees

| Guarantee | Details |
|---|---|
| No cloud upload | Recording files stay on local disk. No auto-upload ever. |
| No telemetry by default | Telemetry is opt-in only, disabled by default |
| No analytics SDKs | No third-party analytics libraries bundled |
| No screen content sent | AI models (Whisper, etc.) run entirely locally |
| Local SQLite only | No remote database or sync by default |

---

## Threat Model

### Assets to Protect

| Asset | Sensitivity | Risk |
|---|---|---|
| Recording files (.mp4) | High | Exfiltration, unauthorized access |
| Microphone audio | High | Capture of private conversations |
| Settings / API keys (future) | Medium | Leakage via settings DB |
| Plugin binaries | Medium | Malicious plugin code execution |

### Threat Actors

| Actor | Capability | Scenario |
|---|---|---|
| Malicious plugin | Code execution in plugin host | Plugin accessing files beyond scope |
| Local attacker | Physical machine access | Reading recording files from disk |
| Supply chain attack | Compromised dependency | FFmpeg DLL substitution |
| Network attacker | MITM on update check | Serving malicious update |

---

## Application Security

### Plugin Sandboxing

Plugins run in a separate `PluginHost.exe` process:
- Communicates with the main process via named pipe IPC only
- Cannot directly access recording engine memory
- File system access limited to declared directories
- Network access denied unless `network` permission granted by user

### Code Signing

All released binaries are signed with an EV code signing certificate:
- `codestudio.exe`
- `codestudio_engine.dll`
- `PluginHost.exe`
- NSIS installer `.exe`

### Update Security

Auto-update mechanism (future):
- Update manifest signed with RSA-2048 private key
- Installer verified with public key bundled in app
- HTTPS only for update server communication
- Hash (SHA-256) of installer verified before execution

---

## Secure Coding Standards

### Memory Safety (C++)

| Rule | Rationale |
|---|---|
| Use `std::unique_ptr` / `std::shared_ptr` | No raw owning pointers |
| No `new`/`delete` in application code | Use RAII containers |
| Use `std::span` for buffer views | Prevent buffer overflows |
| Validate all FFI inputs | Treat Dart input as untrusted |
| Use sanitizers in Debug builds | ASAN, UBSAN enabled in CI Debug builds |

### Input Validation

All configuration values received from Flutter over FFI are validated before use:

```cpp
CaptureResult CaptureController::initialize(const CaptureConfig& config) {
    // Validate all fields
    if (config.fps < 1 || config.fps > 240) {
        return CaptureResult::InvalidConfig;
    }
    if (config.width < 1 || config.width > 7680) {
        return CaptureResult::InvalidConfig;
    }
    if (config.outputPath == nullptr || strlen(config.outputPath) == 0) {
        return CaptureResult::InvalidConfig;
    }
    // Sanitize path — prevent directory traversal
    if (!isAllowedOutputPath(config.outputPath)) {
        return CaptureResult::InvalidConfig;
    }
    // ... proceed
}
```

### Path Traversal Prevention

```cpp
bool isAllowedOutputPath(const char* path) {
    std::filesystem::path p(path);
    p = p.lexically_normal();
    
    // Reject paths with .. components after normalization
    for (auto& part : p) {
        if (part == "..") return false;
    }
    
    // Must be an absolute path on a local drive
    if (!p.is_absolute()) return false;
    
    return true;
}
```

---

## Crash Handling

Unhandled exceptions in the native engine are caught and handled gracefully:

```cpp
SetUnhandledExceptionFilter([](EXCEPTION_POINTERS* ep) -> LONG {
    // Write minidump
    writeMiniDump(ep);
    
    // Try to save partial recording
    SessionManager::instance().emergencyFlush();
    
    // Log crash context
    CrashLogger::log(ep);
    
    return EXCEPTION_EXECUTE_HANDLER;
});
```

Minidump files are saved to `%APPDATA%\CodeStudioRecorder\crashes\` and never uploaded without user consent.

---

## Permissions

### Windows Permissions Required

| Permission | When Requested | Reason |
|---|---|---|
| Microphone access | On first mic recording attempt | WASAPI mic capture |
| Screen capture | Automatic via WGC | No explicit permission needed on Win 10 2004+ |
| File write access | Settings, recordings folder | Saving output |

### No Admin Required

CodeStudio Recorder does not require administrator privileges for normal operation. The installer requests admin (for `PROGRAMFILES` installation), but the running app uses standard user permissions.

---

## Data Retention

| Data | Retention | User Control |
|---|---|---|
| Recording files | Until user deletes | Full delete from UI |
| Recording history (DB) | Soft delete first, hard delete option | Clear history in settings |
| Crash minidumps | 30 days, then auto-deleted | Settings toggle |
| Log files | 7 days rolling | Settings toggle |

---

*Last updated: 2025 | Module 14 of 19*

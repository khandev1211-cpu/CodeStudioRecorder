# 16 — Coding Standards

## CodeStudio Recorder — Coding Standards & Best Practices

---

## Overview

Consistent code is maintainable code. All contributors — human and AI — must follow these standards. Code review enforces these rules before any merge to `main`.

---

## C++ Standards

### Language Version
- C++20 required
- MSVC (Visual Studio 2022) as the primary compiler
- Clang-cl supported for linting/analysis

### Naming Conventions

| Construct | Convention | Example |
|---|---|---|
| Classes | PascalCase | `RecordingEngine`, `AudioMixer` |
| Functions | camelCase | `startRecording()`, `getStats()` |
| Member variables | camelCase + trailing `_` | `frameQueue_`, `isRunning_` |
| Constants | kPascalCase | `kMaxFrameQueueSize` |
| Enums (scoped) | PascalCase values | `RecordingState::Recording` |
| Macros | SCREAMING_SNAKE | `CS_VERSION_MAJOR` |
| Namespaces | lowercase | `namespace cs { }` |
| Files | snake_case | `recording_engine.cpp`, `audio_mixer.h` |

### Formatting

```cpp
// Good
class RecordingEngine {
public:
    explicit RecordingEngine(SessionConfig config);
    
    bool start();
    void stop();
    
    RecordingState state() const { return state_; }

private:
    SessionConfig   config_;
    RecordingState  state_ = RecordingState::Idle;
    FramePipeline   pipeline_;
};
```

- **Indent:** 4 spaces (no tabs)
- **Braces:** same-line opening `{`, new line closing `}`
- **Line length:** ≤ 100 characters
- **Blank lines:** 1 blank line between method definitions in .cpp
- Use `.clang-format` — auto-format on save is required

### Memory Management

```cpp
// ✅ Good — RAII
auto encoder = std::make_unique<NVENCEncoder>();
encoder->initialize(config);

// ❌ Bad — raw owning pointer
NVENCEncoder* encoder = new NVENCEncoder();
```

- Never use raw owning pointers
- Never use `new`/`delete` in application code
- Use `ComPtr<T>` for COM objects
- Use `std::span<T>` for buffer views (not raw pointer + length)

### Error Handling

```cpp
// ✅ Good — return codes for expected failures
EncoderResult NVENCEncoder::initialize(const VideoEncoderConfig& config) {
    if (!probeNVENC()) {
        return EncoderResult::HardwareNotAvailable;
    }
    // ...
    return EncoderResult::OK;
}

// ❌ Bad — exceptions crossing subsystem boundaries
// Exceptions must not cross FFI boundary or subsystem interfaces
```

- Use return codes (`EncoderResult`, `CaptureResult`) for expected failures
- Use `assert()` for internal invariants only (debug builds)
- No exceptions across subsystem boundaries or FFI

---

## Dart / Flutter Standards

### Naming Conventions

| Construct | Convention | Example |
|---|---|---|
| Classes | PascalCase | `RecordingService`, `CsButton` |
| Functions/methods | camelCase | `startRecording()`, `buildHeader()` |
| Variables | camelCase | `currentFps`, `isRecording` |
| Constants | camelCase | `defaultFps`, `maxBitrate` |
| Files | snake_case | `recording_service.dart`, `cs_button.dart` |
| Providers | camelCase + `Provider` suffix | `recordingStateProvider` |
| Riverpod notifiers | PascalCase + `Notifier` | `RecordingNotifier` |

### Widget Guidelines

```dart
// ✅ Good — const constructor, const usage
class CsLabel extends StatelessWidget {
  const CsLabel({super.key, required this.text});
  
  final String text;
  
  @override
  Widget build(BuildContext context) {
    return Text(
      text,
      style: CsTypography.bodyMedium.copyWith(
        color: CsColors.textSecondary,
      ),
    );
  }
}

// ✅ Good — named parameters for readability
CsButton(
  label: 'Start Recording',
  onPressed: _handleRecord,
  style: CsButtonStyle.primary,
)
```

- Always use `const` constructors where possible
- Prefer named parameters over positional for widgets
- `build()` must be pure — no side effects
- Extract sub-widgets if `build()` exceeds 60 lines

### State Management

```dart
// ✅ Good — Riverpod, typed state
final recordingStateProvider = StateNotifierProvider<RecordingNotifier, RecordingState>(
  (ref) => RecordingNotifier(ref.read(recordingServiceProvider)),
);

// ❌ Bad — setState in large widgets, global state
class RecordingScreen extends StatefulWidget {
  // avoid large stateful widgets — use Riverpod providers instead
}
```

---

## Architecture Standards

### Dependency Direction

```
Flutter UI → Application Controller → Engine Subsystems
```

- Lower layers never depend on higher layers
- Subsystems (Capture, Audio, Encoder) never depend on each other directly — only through the Recording Engine coordinator

### Interface First

Every major subsystem is defined as an abstract C++ interface before implementation:

```cpp
// Define first:
class IVideoEncoder { ... };

// Then implement:
class NVENCEncoder : public IVideoEncoder { ... };
class SoftwareEncoder : public IVideoEncoder { ... };
```

### Single Responsibility

Each class does one thing. If a class name contains "and", split it.

- `AudioCaptureMixer` ❌ → `AudioCapture` + `AudioMixer` ✅

---

## Code Review Standards

### PR Requirements

Before requesting review:
- [ ] All tests pass locally
- [ ] `flutter analyze` reports 0 issues
- [ ] `clang-format` applied
- [ ] New code has corresponding tests
- [ ] Documentation updated if public API changed

### Review Checklist

Reviewers check:
- [ ] Architecture: does this follow layering rules?
- [ ] Performance: any allocations in hot paths?
- [ ] Memory: any raw owning pointers? Leaks?
- [ ] Thread safety: any unsynchronized shared state?
- [ ] Error handling: all failure paths handled?
- [ ] Tests: are new code paths covered?

---

## Documentation Standards

### C++ Documentation

```cpp
/// @brief Initializes the capture engine with the given configuration.
///
/// Must be called before start(). Probes capture backends and allocates
/// the texture pool. Thread-safe.
///
/// @param config  Capture configuration (resolution, target, fps)
/// @return CaptureResult::OK on success, error code on failure
CaptureResult initialize(const CaptureConfig& config);
```

### Dart Documentation

```dart
/// Starts a new recording session with the provided [config].
///
/// Throws [RecordingException] if the engine fails to initialize.
/// The [recordingStateProvider] will transition through
/// [RecordingStatus.initializing] → [RecordingStatus.recording].
Future<void> startRecording(RecordingConfig config) async { ... }
```

---

*Last updated: 2025 | Module 16 of 19*

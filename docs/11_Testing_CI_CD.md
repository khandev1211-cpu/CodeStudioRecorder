# 11 — Testing & CI/CD

## CodeStudio Recorder — Testing Strategy & CI/CD Pipeline

---

## Overview

CodeStudio Recorder enforces quality through a layered testing strategy covering unit tests, integration tests, and automated CI/CD via GitHub Actions.

---

## Testing Strategy

### Test Pyramid

```
         [E2E Tests]              ← Few, high-value
        /              \
  [Integration Tests]            ← Subsystem boundaries
    /                    \
[Unit Tests]   [Widget Tests]    ← Many, fast, focused
```

### Test Categories

| Category | Tool | What It Tests | Run Frequency |
|---|---|---|---|
| C++ Unit Tests | Google Test | Engine logic, algorithms | Every commit |
| C++ Integration | Google Test + mocks | Subsystem interactions | Every commit |
| Flutter Unit | flutter_test | Dart business logic | Every commit |
| Flutter Widget | flutter_test | UI component rendering | Every commit |
| E2E (future) | Custom harness | Full recording workflow | Nightly |

---

## C++ Unit Tests

### Test Structure

```
tests/
├── capture/
│   ├── test_frame_queue.cpp
│   ├── test_texture_pool.cpp
│   └── test_capture_controller.cpp
├── audio/
│   ├── test_audio_mixer.cpp
│   ├── test_noise_gate.cpp
│   └── test_resampler.cpp
├── encoder/
│   ├── test_encoder_factory.cpp
│   ├── test_bitrate_controller.cpp
│   └── test_muxer.cpp
├── recording/
│   ├── test_state_machine.cpp
│   ├── test_session_manager.cpp
│   └── test_sync_clock.cpp
└── database/
    ├── test_recording_repository.cpp
    └── test_settings_manager.cpp
```

### Example: State Machine Test

```cpp
TEST(RecordingStateMachineTest, TransitionsFromIdleToRecording) {
    RecordingStateMachine sm;
    EXPECT_EQ(sm.current(), RecordingState::Idle);
    
    EXPECT_TRUE(sm.transition(RecordingEvent::Initialize));
    EXPECT_EQ(sm.current(), RecordingState::Initializing);
    
    EXPECT_TRUE(sm.transition(RecordingEvent::InitOK));
    EXPECT_EQ(sm.current(), RecordingState::Ready);
    
    EXPECT_TRUE(sm.transition(RecordingEvent::Start));
    EXPECT_EQ(sm.current(), RecordingState::Recording);
}

TEST(RecordingStateMachineTest, InvalidTransitionReturnsFalse) {
    RecordingStateMachine sm;
    EXPECT_FALSE(sm.transition(RecordingEvent::Start)); // can't start from Idle
}
```

### Example: Frame Queue Test

```cpp
TEST(SPSCRingBufferTest, PushPopSingleThread) {
    SPSCRingBuffer<int, 8> q;
    
    EXPECT_TRUE(q.push(42));
    int val;
    EXPECT_TRUE(q.pop(val));
    EXPECT_EQ(val, 42);
}

TEST(SPSCRingBufferTest, OverflowDropsOldest) {
    SPSCRingBuffer<int, 4> q;
    for (int i = 0; i < 6; i++) q.push(i);
    // Queue capacity 4 — should have most recent 4
    // ... assert drop counter == 2
}
```

---

## Flutter Tests

### Unit Test Example

```dart
// test/unit/recording_service_test.dart
void main() {
  group('RecordingService', () {
    late MockEngineFFI mockEngine;
    late RecordingService service;
    
    setUp(() {
      mockEngine = MockEngineFFI();
      service = RecordingService(engine: mockEngine);
    });
    
    test('startRecording calls FFI with correct config', () async {
      when(mockEngine.startRecording(any)).thenReturn(0);
      
      final config = RecordingConfig(fps: 60, codec: Codec.h264);
      await service.startRecording(config);
      
      verify(mockEngine.startRecording(
        argThat(isA<NativeRecordingConfig>()
          .having((c) => c.fps, 'fps', 60))
      )).called(1);
    });
  });
}
```

### Widget Test Example

```dart
// test/widget/recording_button_test.dart
void main() {
  testWidgets('CsRecordButton shows red when recording', (tester) async {
    await tester.pumpWidget(
      ProviderScope(
        overrides: [
          recordingStateProvider.overrideWith(
            (ref) => RecordingNotifier.forTest(RecordingStatus.recording),
          ),
        ],
        child: const MaterialApp(home: CsRecordButton()),
      ),
    );
    
    final button = tester.widget<Container>(find.byType(Container).first);
    expect(
      (button.decoration as BoxDecoration).color,
      CsColors.recordRed,
    );
  });
}
```

---

## GitHub Actions CI/CD

### Workflow: `ci.yml`

```yaml
name: CI

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  native-tests:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Configure CMake
        run: cmake -B build -S engine -DCMAKE_BUILD_TYPE=Release
        
      - name: Build Engine
        run: cmake --build build --config Release --parallel
        
      - name: Run C++ Tests
        run: ctest --test-dir build --config Release --output-on-failure

  flutter-tests:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      
      - uses: subosito/flutter-action@v2
        with:
          flutter-version: '3.19.x'
          channel: 'stable'
      
      - run: flutter pub get
      - run: flutter analyze
      - run: flutter test --coverage
      
      - name: Upload coverage
        uses: codecov/codecov-action@v4

  build-release:
    needs: [native-tests, flutter-tests]
    runs-on: windows-latest
    if: github.ref == 'refs/heads/main'
    steps:
      - uses: actions/checkout@v4
      - name: Full Release Build
        run: powershell scripts/build.ps1 -Config Release -Package
      
      - uses: actions/upload-artifact@v4
        with:
          name: installer
          path: dist/*.exe
```

### Release Workflow: `release.yml`

```yaml
name: Release

on:
  push:
    tags: ['v*']

jobs:
  release:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Build
        run: powershell scripts/build.ps1 -Config Release -Package
      
      - name: Sign Binaries
        env:
          CERT_PFX: ${{ secrets.SIGNING_CERT }}
          CERT_PASSWORD: ${{ secrets.SIGNING_CERT_PASSWORD }}
        run: powershell scripts/sign.ps1
      
      - name: Create GitHub Release
        uses: softprops/action-gh-release@v2
        with:
          files: |
            dist/CodeStudioRecorder-Setup-*.exe
            dist/CodeStudioRecorder-*-portable.zip
          draft: true
          generate_release_notes: true
```

---

## Quality Gates

Pull requests must pass all of these before merge:

| Gate | Threshold |
|---|---|
| All C++ tests | 100% pass |
| All Flutter tests | 100% pass |
| Flutter analysis | 0 errors |
| Code coverage (Dart) | ≥ 80% |
| Build (Release) | Successful |
| Binary size | < 80MB installer |

---

## Performance Regression Tests

Run nightly to catch encoding/capture performance regressions:

```cpp
TEST(PerformanceBenchmark, EncodeFrameUnder2Ms) {
    auto encoder = createTestEncoder(1920, 1080, 60);
    auto frame   = createTestFrame(1920, 1080);
    
    auto start = std::chrono::high_resolution_clock::now();
    encoder->encodeFrame(frame);
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    
    EXPECT_LT(
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(),
        2
    );
}
```

---

*Last updated: 2025 | Module 11 of 19*

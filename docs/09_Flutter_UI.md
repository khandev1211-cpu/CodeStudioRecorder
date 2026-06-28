# 09 — Flutter UI

## CodeStudio Recorder — Flutter UI Architecture

---

## Table of Contents

1. [Overview](#overview)
2. [Application Architecture](#application-architecture)
3. [Navigation Structure](#navigation-structure)
4. [State Management](#state-management)
5. [Core Widgets](#core-widgets)
6. [Themes](#themes)
7. [Localization](#localization)
8. [Responsive Layouts](#responsive-layouts)
9. [FFI Bridge Layer](#ffi-bridge-layer)
10. [Design System](#design-system)

---

## Overview

The Flutter UI is the user-facing layer of CodeStudio Recorder. It is built with Flutter Desktop for Windows, using Dart as the language. The UI is intentionally lightweight — it acts as a configuration and status layer; all heavy work is done in the native C++ engine.

**Core philosophy:**
- Minimal surface area during recording (overlay only)
- Rich configuration experience when setting up
- Instant feedback on all actions
- No janky animations — 60fps smooth always

---

## Application Architecture

```
lib/
├── main.dart                  ← App entry point
├── app.dart                   ← MaterialApp / routing setup
│
├── core/
│   ├── ffi/                   ← FFI bridge to native engine
│   │   ├── engine_bindings.dart
│   │   ├── recording_ffi.dart
│   │   └── types/
│   ├── services/              ← App-level services
│   │   ├── recording_service.dart
│   │   ├── settings_service.dart
│   │   └── export_service.dart
│   ├── models/                ← Data models (Dart)
│   │   ├── recording.dart
│   │   ├── export.dart
│   │   └── profile.dart
│   └── utils/
│       ├── file_utils.dart
│       └── time_utils.dart
│
├── features/
│   ├── home/                  ← Home screen
│   ├── recording/             ← Recording control overlay
│   ├── history/               ← Recording history
│   ├── export/                ← Export panel
│   ├── settings/              ← Settings pages
│   ├── profiles/              ← Recording profiles
│   └── plugins/               ← Plugin manager
│
├── shared/
│   ├── widgets/               ← Reusable components
│   │   ├── cs_button.dart
│   │   ├── cs_toggle.dart
│   │   ├── cs_slider.dart
│   │   ├── cs_dropdown.dart
│   │   └── cs_tooltip.dart
│   └── theme/
│       ├── cs_theme.dart
│       ├── cs_colors.dart
│       └── cs_typography.dart
```

---

## Navigation Structure

CodeStudio Recorder uses a **side-rail + content area** layout for the main window.

```
┌─────────────────────────────────────────────────────────┐
│  ● ● ●   CodeStudio Recorder                   [_][□][X]│
├──────────┬──────────────────────────────────────────────┤
│          │                                              │
│  [Home]  │              Content Area                   │
│  [Hist]  │                                              │
│  [Export]│                                              │
│  [Plug]  │                                              │
│          │                                              │
│  [Set]   │                                              │
│  [Help]  │                                              │
└──────────┴──────────────────────────────────────────────┘
```

### Routes

```dart
final routes = GoRouter(
  routes: [
    GoRoute(
      path: '/',
      builder: (context, state) => const HomeScreen(),
    ),
    GoRoute(
      path: '/recording',
      builder: (context, state) => const RecordingOverlay(),
    ),
    GoRoute(
      path: '/history',
      builder: (context, state) => const HistoryScreen(),
    ),
    GoRoute(
      path: '/history/:id',
      builder: (context, state) => RecordingDetailScreen(
        id: state.pathParameters['id']!,
      ),
    ),
    GoRoute(
      path: '/export/:id',
      builder: (context, state) => ExportScreen(
        recordingId: state.pathParameters['id']!,
      ),
    ),
    GoRoute(
      path: '/settings',
      builder: (context, state) => const SettingsScreen(),
    ),
    GoRoute(
      path: '/plugins',
      builder: (context, state) => const PluginManagerScreen(),
    ),
  ],
);
```

---

## State Management

CodeStudio Recorder uses **Riverpod** for state management.

### Why Riverpod

- Compile-time safe — no `context.watch<T>()` string mistakes
- No BuildContext required for business logic
- Clean separation of state from UI
- Works well with async FFI calls

### Provider Structure

```dart
// Recording state
final recordingStateProvider = StateNotifierProvider<RecordingNotifier, RecordingState>(
  (ref) => RecordingNotifier(ref.read(recordingServiceProvider)),
);

// Settings
final settingsProvider = FutureProvider<AppSettings>(
  (ref) => ref.read(settingsServiceProvider).load(),
);

// History
final recordingHistoryProvider = FutureProvider.family<List<Recording>, HistoryFilter>(
  (ref, filter) => ref.read(historyRepositoryProvider).findAll(filter: filter),
);

// Active recording stats
final recordingStatsProvider = StreamProvider<RecordingStats>(
  (ref) => ref.read(recordingServiceProvider).statsStream,
);
```

### RecordingState

```dart
@freezed
class RecordingState with _$RecordingState {
  const factory RecordingState({
    @Default(RecordingStatus.idle) RecordingStatus status,
    Duration? elapsedTime,
    int? droppedFrames,
    double? encoderLoad,
    String? errorMessage,
    String? currentFilePath,
  }) = _RecordingState;
}

enum RecordingStatus { idle, initializing, ready, recording, paused, flushing, error }
```

---

## Core Widgets

### CsButton

```dart
class CsButton extends StatelessWidget {
  final String label;
  final VoidCallback? onPressed;
  final CsButtonStyle style;
  final Widget? icon;
  final bool loading;

  // Styles: primary, secondary, ghost, danger
}
```

### CsRecordButton

The main record/stop button with animated state:

```dart
class CsRecordButton extends ConsumerWidget {
  // Watches recordingStateProvider
  // Red pulsing circle when recording
  // Clean button when idle
  // Spinner when initializing/flushing
}
```

### CsTimerDisplay

```dart
class CsTimerDisplay extends ConsumerWidget {
  // Displays HH:MM:SS from recordingStatsProvider
  // Monospace font for stable layout
  // Red tint when recording
}
```

### CsAudioMeter

Real-time audio level meter:

```dart
class CsAudioMeter extends ConsumerWidget {
  final AudioSource source; // mic or system

  // Displays dBFS level from audioStatsStream
  // Green / yellow / red zones
  // Peak hold indicator
}
```

### CsRecordingCard

History list item:

```dart
class CsRecordingCard extends StatelessWidget {
  final Recording recording;
  final VoidCallback? onTap;
  final VoidCallback? onExport;
  final VoidCallback? onDelete;

  // Shows thumbnail, title, duration, date, size
  // Context menu on right-click
}
```

---

## Themes

CodeStudio Recorder ships with **dark** and **light** themes, defaulting to dark.

### Dark Theme (Default)

```dart
final darkTheme = ThemeData(
  brightness: Brightness.dark,
  colorScheme: const ColorScheme.dark(
    primary:   Color(0xFFFF3B3B),   // CodeStudio Red
    secondary: Color(0xFF2979FF),   // Accent Blue
    surface:   Color(0xFF1A1A1A),   // Card surface
    background:Color(0xFF111111),   // App background
    onPrimary: Color(0xFFFFFFFF),
    onSurface: Color(0xFFE8E8E8),
  ),
  // ...
);
```

### Design Tokens

```dart
abstract class CsColors {
  static const recordRed    = Color(0xFFFF3B3B);
  static const accentBlue   = Color(0xFF2979FF);
  static const success      = Color(0xFF4CAF50);
  static const warning      = Color(0xFFFFC107);
  static const danger       = Color(0xFFFF5252);
  
  // Surfaces
  static const surface100   = Color(0xFF111111);  // background
  static const surface200   = Color(0xFF1A1A1A);  // card
  static const surface300   = Color(0xFF242424);  // elevated card
  static const surface400   = Color(0xFF2E2E2E);  // input, hover
  
  // Text
  static const textPrimary  = Color(0xFFEEEEEE);
  static const textSecond   = Color(0xFF9E9E9E);
  static const textDisabled = Color(0xFF616161);
}
```

---

## Localization

CodeStudio Recorder uses `flutter_localizations` and ARB files for internationalization.

```
lib/l10n/
├── app_en.arb    ← English (default)
├── app_es.arb    ← Spanish
├── app_de.arb    ← German
├── app_fr.arb    ← French
├── app_ja.arb    ← Japanese
└── app_zh.arb    ← Chinese (Simplified)
```

### Usage

```dart
// In widget:
Text(context.l10n.recordingStarted)

// app_en.arb:
{
  "recordingStarted": "Recording started",
  "recordingStarted@type": "text",
  
  "durationLabel": "Duration: {duration}",
  "durationLabel@placeholders": {
    "duration": { "type": "String" }
  }
}
```

---

## Responsive Layouts

The main window is resizable. UI adapts to window size:

| Window Width | Layout |
|---|---|
| < 600px | Compact: hide labels, icon-only rail |
| 600–900px | Normal: labels visible, single-column content |
| > 900px | Wide: two-column content on some screens |

```dart
class AdaptiveLayout extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    final width = MediaQuery.of(context).size.width;
    
    if (width < 600) return const CompactLayout();
    if (width < 900) return const NormalLayout();
    return const WideLayout();
  }
}
```

---

## FFI Bridge Layer

All communication from Flutter to the native C++ engine goes through the FFI bridge.

```dart
// lib/core/ffi/recording_ffi.dart

final _engine = DynamicLibrary.open('codestudio_engine.dll');

// Bind native functions
final _startRecording = _engine.lookupFunction<
  Int32 Function(Pointer<NativeRecordingConfig>),
  int    Function(Pointer<NativeRecordingConfig>)
>('cse_start_recording');

final _stopRecording = _engine.lookupFunction<
  Int32 Function(),
  int    Function()
>('cse_stop_recording');

final _getStats = _engine.lookupFunction<
  Void Function(Pointer<NativeRecordingStats>),
  void  Function(Pointer<NativeRecordingStats>)
>('cse_get_stats');
```

### RecordingService (Dart)

Wraps the FFI calls with async, error handling, and state updates:

```dart
class RecordingService {
  Future<void> startRecording(RecordingConfig config) async {
    final nativeConfig = config.toNative();
    final result = _startRecording(nativeConfig);
    nativeConfig.free();
    
    if (result != 0) {
      throw RecordingException.fromCode(result);
    }
  }
  
  Stream<RecordingStats> get statsStream => 
    Stream.periodic(const Duration(milliseconds: 100))
      .map((_) => _pollStats());
}
```

---

## Design System

### Typography Scale

```dart
abstract class CsTypography {
  static const displayLarge  = TextStyle(fontSize: 32, fontWeight: FontWeight.w700);
  static const displayMedium = TextStyle(fontSize: 24, fontWeight: FontWeight.w600);
  static const titleLarge    = TextStyle(fontSize: 18, fontWeight: FontWeight.w600);
  static const titleMedium   = TextStyle(fontSize: 16, fontWeight: FontWeight.w500);
  static const bodyLarge     = TextStyle(fontSize: 14, fontWeight: FontWeight.w400);
  static const bodyMedium    = TextStyle(fontSize: 13, fontWeight: FontWeight.w400);
  static const labelLarge    = TextStyle(fontSize: 12, fontWeight: FontWeight.w500);
  static const mono          = TextStyle(fontSize: 13, fontFamily: 'JetBrains Mono');
}
```

### Spacing Scale

```dart
abstract class CsSpacing {
  static const double xs  = 4.0;
  static const double sm  = 8.0;
  static const double md  = 16.0;
  static const double lg  = 24.0;
  static const double xl  = 32.0;
  static const double xxl = 48.0;
}
```

---

*Last updated: 2025 | Module 09 of 19*

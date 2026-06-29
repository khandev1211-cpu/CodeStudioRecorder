import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'package:codestudio_recorder/core/ffi/engine_bindings.dart';
import 'package:codestudio_recorder/core/ffi/types/native_types.dart';
import 'package:codestudio_recorder/core/models/recording_stats.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

final recordingServiceProvider = Provider((ref) => RecordingService());

final List<({int hwnd, String title})> _tempWindowsList = [];

void _onWindowEnumerated(Pointer<NativeWindowInfo> info) {
  final hwnd = info.ref.hwnd;
  final titlePtr = info.ref.title.cast<Utf8>();
  final String title = titlePtr.toDartString();
  _tempWindowsList.add((hwnd: hwnd, title: title));
}

class RecordingService {
  late final EngineBindings _bindings;
  bool _isInitialized = false;

  RecordingService() {
    try {
      _bindings = EngineBindings();
      _isInitialized = true;
    } catch (e) {
      print("CRITICAL: Failed to load native engine: $e");
    }
  }

  bool get isInitialized => _isInitialized;

  int start(int width, int height, int fps, String outputPath, int targetHwnd) {
    if (!_isInitialized) return -999;

    final configPtr = calloc<NativeRecordingConfig>();
    final pathPtr = outputPath.toNativeUtf8();
    
    try {
      configPtr.ref.width = width;
      configPtr.ref.height = height;
      configPtr.ref.fps = fps;
      configPtr.ref.outputPath = pathPtr;
      configPtr.ref.captureCursor = true;
      configPtr.ref.captureAudio = true;
      configPtr.ref.targetHwnd = targetHwnd;

      return _bindings.startRecording(configPtr);
    } finally {
      // Memory management: The engine copies strings on start, so we can free here.
      // But we free the pointers in a real app after ensuring the engine is done.
      // For MVP, we'll keep them for the duration of the call.
    }
  }

  int stop() => _isInitialized ? _bindings.stopRecording() : -999;
  int pause() => _isInitialized ? _bindings.pauseRecording() : -999;
  int resume() => _isInitialized ? _bindings.resumeRecording() : -999;

  RecordingStatus get status => _isInitialized 
    ? RecordingStatus.fromInt(_bindings.getStatus()) 
    : RecordingStatus.error;

  RecordingStats getStats() {
    if (!_isInitialized) return RecordingStats(elapsedMs: 0, droppedFrames: 0, encoderLoad: 0);
    
    final statsPtr = calloc<NativeRecordingStats>();
    try {
      _bindings.getStats(statsPtr);
      return RecordingStats(
        elapsedMs: statsPtr.ref.elapsedMs,
        droppedFrames: statsPtr.ref.droppedFrames,
        encoderLoad: statsPtr.ref.encoderLoad,
      );
    } finally {
      calloc.free(statsPtr);
    }
  }

  List<({int hwnd, String title})> getWindows() {
    if (!_isInitialized) return [];
    _tempWindowsList.clear();
    final nativeCallback = Pointer.fromFunction<WindowCallbackNative>(_onWindowEnumerated);
    _bindings.enumerateWindows(nativeCallback);
    return List.from(_tempWindowsList);
  }

  void setSettingString(String key, String value) {
    if (!_isInitialized) return;
    _bindings.setSettingString(key.toNativeUtf8(), value.toNativeUtf8());
  }

  String getSettingString(String key, String defaultValue) {
    if (!_isInitialized) return defaultValue;
    final result = _bindings.getSettingString(key.toNativeUtf8(), defaultValue.toNativeUtf8());
    return result.cast<Utf8>().toDartString();
  }

  void setSettingInt(String key, int value) {
    if (!_isInitialized) return;
    _bindings.setSettingInt(key.toNativeUtf8(), value);
  }

  int getSettingInt(String key, int defaultValue) {
    if (!_isInitialized) return defaultValue;
    return _bindings.getSettingInt(key.toNativeUtf8(), defaultValue);
  }

  void setProcessorEnabled(int index, bool enabled) {
    if (!_isInitialized) return;
    _bindings.setProcessorEnabled(index, enabled);
  }

  void reportMouseClick(double x, double y) {
    if (!_isInitialized) return;
    _bindings.reportMouseClick(x, y);
  }
}

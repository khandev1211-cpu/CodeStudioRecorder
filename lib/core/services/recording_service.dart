import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'package:codestudio_recorder/core/ffi/engine_bindings.dart';
import 'package:codestudio_recorder/core/ffi/types/native_types.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

final recordingServiceProvider = Provider((ref) => RecordingService());

final List<({int hwnd, String title})> _enumeratedWindows = [];

void _windowCallback(NativeWindowInfo info) {
  _enumeratedWindows.add((hwnd: info.hwnd, title: info.title.toDartString()));
}

class RecordingService {
  final EngineBindings _bindings = EngineBindings();

  int start(int width, int height, int fps, String outputPath, int targetHwnd) {
    final config = calloc<NativeRecordingConfig>();
    config.ref.width = width;
    config.ref.height = height;
    config.ref.fps = fps;
    config.ref.outputPath = outputPath.toNativeUtf8();
    config.ref.captureCursor = true;
    config.ref.captureAudio = true;
    config.ref.targetHwnd = targetHwnd;

    final result = _bindings.startRecording(config);
    return result;
  }

  int stop() => _bindings.stopRecording();
  int pause() => _bindings.pauseRecording();
  int resume() => _bindings.resumeRecording();

  RecordingStatus get status => RecordingStatus.fromInt(_bindings.getStatus());

  NativeRecordingStats getStats() {
    final stats = calloc<NativeRecordingStats>();
    _bindings.getStats(stats);
    return stats.ref;
  }

  List<({int hwnd, String title})> getWindows() {
    _enumeratedWindows.clear();
    final nativeCallback = Pointer.fromFunction<WindowCallbackNative>(_windowCallback);
    _bindings.enumerateWindows(nativeCallback);
    return List.from(_enumeratedWindows);
  }

  void setSettingString(String key, String value) {
    _bindings.setSettingString(key.toNativeUtf8(), value.toNativeUtf8());
  }

  String getSettingString(String key, String defaultValue) {
    final result = _bindings.getSettingString(key.toNativeUtf8(), defaultValue.toNativeUtf8());
    return result.toDartString();
  }

  void setSettingInt(String key, int value) {
    _bindings.setSettingInt(key.toNativeUtf8(), value);
  }

  int getSettingInt(String key, int defaultValue) {
    return _bindings.getSettingInt(key.toNativeUtf8(), defaultValue);
  }
}

import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'package:codestudio_recorder/core/ffi/engine_bindings.dart';
import 'package:codestudio_recorder/core/ffi/types/native_types.dart';
import 'package:codestudio_recorder/core/models/recording_stats.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

final recordingServiceProvider = Provider((ref) => RecordingService());

// Shared list for window enumeration
final List<({int hwnd, String title})> _enumeratedWindowsList = [];

// Static callback as required by dart:ffi
void _onWindowEnumerated(Pointer<NativeWindowInfo> info) {
  final hwnd = info.ref.hwnd;
  final titlePtr = info.ref.title.cast<Utf8>();
  final String title = titlePtr.toDartString();
  _enumeratedWindowsList.add((hwnd: hwnd, title: title));
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
    // Note: We don't free config here if the engine needs it persistently,
    // but typically the engine copies the data.
    return result;
  }

  int stop() => _bindings.stopRecording();
  int pause() => _bindings.pauseRecording();
  int resume() => _bindings.resumeRecording();

  RecordingStatus get status => RecordingStatus.fromInt(_bindings.getStatus());

  RecordingStats getStats() {
    final statsPtr = calloc<NativeRecordingStats>();
    _bindings.getStats(statsPtr);
    final stats = RecordingStats(
      elapsedMs: statsPtr.ref.elapsedMs,
      droppedFrames: statsPtr.ref.droppedFrames,
      encoderLoad: statsPtr.ref.encoderLoad,
    );
    calloc.free(statsPtr);
    return stats;
  }

  List<({int hwnd, String title})> getWindows() {
    _enumeratedWindowsList.clear();
    final nativeCallback = Pointer.fromFunction<WindowCallbackNative>(_onWindowEnumerated);
    _bindings.enumerateWindows(nativeCallback);
    return List.from(_enumeratedWindowsList);
  }

  void setSettingString(String key, String value) {
    _bindings.setSettingString(key.toNativeUtf8(), value.toNativeUtf8());
  }

  String getSettingString(String key, String defaultValue) {
    final result = _bindings.getSettingString(
      key.toNativeUtf8(),
      defaultValue.toNativeUtf8(),
    );
    return result.cast<Utf8>().toDartString();
  }

  void setSettingInt(String key, int value) {
    _bindings.setSettingInt(key.toNativeUtf8(), value);
  }

  int getSettingInt(String key, int defaultValue) {
    return _bindings.getSettingInt(key.toNativeUtf8(), defaultValue);
  }

  void setProcessorEnabled(int index, bool enabled) {
    _bindings.setProcessorEnabled(index, enabled);
  }
}

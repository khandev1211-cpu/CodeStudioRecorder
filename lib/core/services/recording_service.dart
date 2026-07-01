import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'package:codestudio_recorder/core/ffi/engine_bindings.dart';
import 'package:codestudio_recorder/core/ffi/types/native_types.dart';
import 'package:codestudio_recorder/core/models/recording_stats.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

final recordingServiceProvider = Provider((ref) => RecordingService());

final List<({int hwnd, String title})> _tempWindowsList = [];
final List<({String id, String name, bool isDefault})> _tempAudioList = [];
final List<({String id, String name})> _tempWebcamList = [];

void _onWindowEnumerated(Pointer<NativeWindowInfo> info) {
  final hwnd = info.ref.hwnd;
  final titlePtr = info.ref.title.cast<Utf8>();
  final String title = titlePtr.toDartString();
  _tempWindowsList.add((hwnd: hwnd, title: title));
}

void _onAudioDeviceEnumerated(Pointer<NativeAudioDeviceInfo> info) {
  final id = info.ref.id.cast<Utf8>().toDartString();
  final name = info.ref.name.cast<Utf8>().toDartString();
  _tempAudioList.add((id: id, name: name, isDefault: info.ref.isDefault));
}

void _onWebcamEnumerated(Pointer<NativeWebcamDeviceInfo> info) {
  final id = info.ref.id.cast<Utf8>().toDartString();
  final name = info.ref.name.cast<Utf8>().toDartString();
  _tempWebcamList.add((id: id, name: name));
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

  int start(int width, int height, int fps, String outputPath, int targetHwnd, String encoder, {String? micId, String? sysId, String? webcamId}) {
    if (!_isInitialized) return -999;

    final configPtr = calloc<NativeRecordingConfig>();
    final pathPtr = outputPath.toNativeUtf8();
    final encoderPtr = encoder.toNativeUtf8();
    final micIdPtr = (micId ?? "").toNativeUtf8();
    final sysIdPtr = (sysId ?? "").toNativeUtf8();
    final webcamIdPtr = (webcamId ?? "").toNativeUtf8();
    
    try {
      configPtr.ref.width = width;
      configPtr.ref.height = height;
      configPtr.ref.fps = fps;
      configPtr.ref.outputPath = pathPtr;
      configPtr.ref.captureCursor = true;
      configPtr.ref.captureAudio = true;
      configPtr.ref.targetHwnd = targetHwnd;
      configPtr.ref.encoderName = encoderPtr;
      configPtr.ref.micDeviceId = micIdPtr;
      configPtr.ref.sysAudioDeviceId = sysIdPtr;
      configPtr.ref.webcamDeviceId = webcamIdPtr;

      return _bindings.startRecording(configPtr);
    } finally {
      // Memory management: The engine copies strings on start
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

  ({double mic, double system}) getAudioLevels() {
    if (!_isInitialized) return (mic: 0.0, system: 0.0);
    final micPtr = calloc<Float>();
    final sysPtr = calloc<Float>();
    try {
      _bindings.getAudioLevels(micPtr, sysPtr);
      return (mic: micPtr.value, system: sysPtr.value);
    } finally {
      calloc.free(micPtr);
      calloc.free(sysPtr);
    }
  }

  List<({int hwnd, String title})> getWindows() {
    if (!_isInitialized) return [];
    _tempWindowsList.clear();
    final nativeCallback = Pointer.fromFunction<WindowCallbackNative>(_onWindowEnumerated);
    _bindings.enumerateWindows(nativeCallback);
    return List.from(_tempWindowsList);
  }

  List<({String id, String name, bool isDefault})> getAudioDevices(bool capture) {
    if (!_isInitialized) return [];
    _tempAudioList.clear();
    final nativeCallback = Pointer.fromFunction<AudioDeviceCallbackNative>(_onAudioDeviceEnumerated);
    _bindings.enumerateAudioDevices(capture, nativeCallback);
    return List.from(_tempAudioList);
  }

  List<({String id, String name})> getWebcams() {
    if (!_isInitialized) return [];
    _tempWebcamList.clear();
    final nativeCallback = Pointer.fromFunction<WebcamCallbackNative>(_onWebcamEnumerated);
    _bindings.enumerateWebcams(nativeCallback);
    return List.from(_tempWebcamList);
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

  void setPluginEnabled(int index, bool enabled) {
    if (!_isInitialized) return;
    _bindings.setPluginEnabled(index, enabled);
  }

  int getPluginCount() {
    if (!_isInitialized) return 0;
    return _bindings.getPluginCount();
  }

  void reportMouseClick(double x, double y) {
    if (!_isInitialized) return;
    _bindings.reportMouseClick(x, y);
  }

  void addAnnotation(int type, double x1, double y1, double x2, double y2, int color, double width) {
    if (!_isInitialized) return;
    _bindings.addAnnotation(type, x1, y1, x2, y2, color, width);
  }

  void clearAnnotations() {
    if (!_isInitialized) return;
    _bindings.clearAnnotations();
  }

  void undoAnnotation() {
    if (!_isInitialized) return;
    _bindings.undoAnnotation();
  }

  void setZoomLevel(double level) {
    if (!_isInitialized) return;
    _bindings.setZoomLevel(level);
  }

  void setWebcamPosition(double x, double y, double width, double height) {
    if (!_isInitialized) return;
    _bindings.setWebcamPosition(x, y, width, height);
  }

  void addChapterMarker(String label) {
    if (!_isInitialized) return;
    _bindings.addChapterMarker(label.toNativeUtf8());
  }
}

import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'package:codestudio_recorder/core/ffi/engine_bindings.dart';
import 'package:codestudio_recorder/core/ffi/types/native_types.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

final recordingServiceProvider = Provider((ref) => RecordingService());

class RecordingService {
  final EngineBindings _bindings = EngineBindings();

  int start(int width, int height, int fps, String outputPath) {
    final config = calloc<NativeRecordingConfig>();
    config.ref.width = width;
    config.ref.height = height;
    config.ref.fps = fps;
    config.ref.outputPath = outputPath.toNativeUtf8();
    config.ref.captureCursor = true;
    config.ref.captureAudio = true;

    final result = _bindings.startRecording(config);
    
    // In a real app, we'd manage the memory more carefully, 
    // maybe freeing the string after the call if the engine copies it.
    // calloc.free(config.ref.outputPath);
    // calloc.free(config);
    
    return result;
  }

  int stop() => _bindings.stopRecording();
  int pause() => _bindings.pauseRecording();
  int resume() => _bindings.resumeRecording();

  RecordingStatus get status => RecordingStatus.fromInt(_bindings.getStatus());

  NativeRecordingStats getStats() {
    final stats = calloc<NativeRecordingStats>();
    _bindings.getStats(stats);
    final result = stats.ref;
    // Note: This is simplified. Normally we'd copy the values and free the pointer.
    return result;
  }
}

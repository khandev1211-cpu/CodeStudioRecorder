import 'dart:ffi';
import 'dart:io';
import 'package:codestudio_recorder/core/ffi/types/native_types.dart';

typedef StartRecordingNative = Int32 Function(Pointer<NativeRecordingConfig>);
typedef StartRecordingDart = int Function(Pointer<NativeRecordingConfig>);

typedef StopRecordingNative = Int32 Function();
typedef StopRecordingDart = int Function();

typedef GetStatsNative = Void Function(Pointer<NativeRecordingStats>);
typedef GetStatsDart = void Function(Pointer<NativeRecordingStats>);

typedef GetStatusNative = Int32 Function();
typedef GetStatusDart = int Function();

class EngineBindings {
  late final DynamicLibrary _lib;

  late final StartRecordingDart startRecording;
  late final StopRecordingDart stopRecording;
  late final StopRecordingDart pauseRecording;
  late final StopRecordingDart resumeRecording;
  late final GetStatsDart getStats;
  late final GetStatusDart getStatus;

  EngineBindings() {
    _lib = _loadLibrary();
    _initializeBindings();
  }

  DynamicLibrary _loadLibrary() {
    if (Platform.isWindows) {
      return DynamicLibrary.open('codestudio_engine.dll');
    }
    throw UnsupportedError('Platform not supported');
  }

  void _initializeBindings() {
    startRecording = _lib
        .lookup<NativeFunction<StartRecordingNative>>('cse_start_recording')
        .asFunction();
    stopRecording = _lib
        .lookup<NativeFunction<StopRecordingNative>>('cse_stop_recording')
        .asFunction();
    pauseRecording = _lib
        .lookup<NativeFunction<StopRecordingNative>>('cse_pause_recording')
        .asFunction();
    resumeRecording = _lib
        .lookup<NativeFunction<StopRecordingNative>>('cse_resume_recording')
        .asFunction();
    getStats = _lib
        .lookup<NativeFunction<GetStatsNative>>('cse_get_stats')
        .asFunction();
    getStatus = _lib
        .lookup<NativeFunction<GetStatusNative>>('cse_get_status')
        .asFunction();
  }
}

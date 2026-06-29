import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';
import 'package:codestudio_recorder/core/ffi/types/native_types.dart';

typedef StartRecordingNative = Int32 Function(Pointer<NativeRecordingConfig>);
typedef StartRecordingDart = int Function(Pointer<NativeRecordingConfig>);

typedef StopRecordingNative = Int32 Function();
typedef StopRecordingDart = int Function();

typedef GetStatsNative = Void Function(Pointer<NativeRecordingStats>);
typedef GetStatsDart = void Function(Pointer<NativeRecordingStats>);

typedef GetStatusNative = Int32 Function();
typedef GetStatusDart = int Function();

typedef WindowCallbackNative = Void Function(Pointer<NativeWindowInfo>);
typedef EnumerateWindowsNative = Void Function(Pointer<NativeFunction<WindowCallbackNative>>);
typedef EnumerateWindowsDart = void Function(Pointer<NativeFunction<WindowCallbackNative>>);

typedef SetSettingStringNative = Void Function(Pointer<Utf8>, Pointer<Utf8>);
typedef SetSettingStringDart = void Function(Pointer<Utf8>, Pointer<Utf8>);

typedef GetSettingStringNative = Pointer<Utf8> Function(Pointer<Utf8>, Pointer<Utf8>);
typedef GetSettingStringDart = Pointer<Utf8> Function(Pointer<Utf8>, Pointer<Utf8>);

typedef SetSettingIntNative = Void Function(Pointer<Utf8>, Int32);
typedef SetSettingIntDart = void Function(Pointer<Utf8>, int);

typedef GetSettingIntNative = Int32 Function(Pointer<Utf8>, Int32);
typedef GetSettingIntDart = int Function(Pointer<Utf8>, int);

typedef SetProcessorEnabledNative = Void Function(Int32, Bool);
typedef SetProcessorEnabledDart = void Function(int, bool);

typedef ReportMouseClickNative = Void Function(Float, Float);
typedef ReportMouseClickDart = void Function(double, double);

typedef AddAnnotationNative = Void Function(Int32, Float, Float, Float, Float, Uint32, Float);
typedef AddAnnotationDart = void Function(int, double, double, double, double, int, double);

typedef ClearAnnotationsNative = Void Function();
typedef ClearAnnotationsDart = void Function();

typedef UndoAnnotationNative = Void Function();
typedef UndoAnnotationDart = void Function();

typedef SetZoomLevelNative = Void Function(Float);
typedef SetZoomLevelDart = void Function(double);

typedef SetWebcamPositionNative = Void Function(Float, Float, Float, Float);
typedef SetWebcamPositionDart = void Function(double, double, double, double);

class EngineBindings {
  late final DynamicLibrary _lib;

  late final StartRecordingDart startRecording;
  late final StopRecordingDart stopRecording;
  late final StopRecordingDart pauseRecording;
  late final StopRecordingDart resumeRecording;
  late final GetStatsDart getStats;
  late final GetStatusDart getStatus;
  late final EnumerateWindowsDart enumerateWindows;
  
  late final SetSettingStringDart setSettingString;
  late final GetSettingStringDart getSettingString;
  late final SetSettingIntDart setSettingInt;
  late final GetSettingIntDart getSettingInt;
  late final SetProcessorEnabledDart setProcessorEnabled;
  late final ReportMouseClickDart reportMouseClick;
  late final AddAnnotationDart addAnnotation;
  late final ClearAnnotationsDart clearAnnotations;
  late final UndoAnnotationDart undoAnnotation;
  late final SetZoomLevelDart setZoomLevel;
  late final SetWebcamPositionDart setWebcamPosition;

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
    enumerateWindows = _lib
        .lookup<NativeFunction<EnumerateWindowsNative>>('cse_enumerate_windows')
        .asFunction();
    setSettingString = _lib
        .lookup<NativeFunction<SetSettingStringNative>>('cse_set_setting_string')
        .asFunction();
    getSettingString = _lib
        .lookup<NativeFunction<GetSettingStringNative>>('cse_get_setting_string')
        .asFunction();
    setSettingInt = _lib
        .lookup<NativeFunction<SetSettingIntNative>>('cse_set_setting_int')
        .asFunction();
    getSettingInt = _lib
        .lookup<NativeFunction<GetSettingIntNative>>('cse_get_setting_int')
        .asFunction();
    setProcessorEnabled = _lib
        .lookup<NativeFunction<SetProcessorEnabledNative>>('cse_set_processor_enabled')
        .asFunction();
    reportMouseClick = _lib
        .lookup<NativeFunction<ReportMouseClickNative>>('cse_report_mouse_click')
        .asFunction();
    addAnnotation = _lib
        .lookup<NativeFunction<AddAnnotationNative>>('cse_add_annotation')
        .asFunction();
    clearAnnotations = _lib
        .lookup<NativeFunction<ClearAnnotationsNative>>('cse_clear_annotations')
        .asFunction();
    undoAnnotation = _lib
        .lookup<NativeFunction<UndoAnnotationNative>>('cse_undo_annotation')
        .asFunction();
    setZoomLevel = _lib
        .lookup<NativeFunction<SetZoomLevelNative>>('cse_set_zoom_level')
        .asFunction();
    setWebcamPosition = _lib
        .lookup<NativeFunction<SetWebcamPositionNative>>('cse_set_webcam_position')
        .asFunction();
  }
}

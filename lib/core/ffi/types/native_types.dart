import 'dart:ffi';
import 'package:ffi/ffi.dart';

enum RecordingStatus {
  idle,
  initializing,
  ready,
  recording,
  paused,
  flushing,
  finalizing,
  completed,
  error;

  static RecordingStatus fromInt(int value) => RecordingStatus.values[value];
}

final class NativeRecordingConfig extends Struct {
  @Uint32()
  external int width;
  @Uint32()
  external int height;
  @Uint32()
  external int fps;
  external Pointer<Utf8> outputPath;
  @Bool()
  external bool captureCursor;
  @Bool()
  external bool captureAudio;
  @Int64()
  external int targetHwnd;
}

final class NativeRecordingStats extends Struct {
  @Int64()
  external int elapsedMs;
  @Uint32()
  external int droppedFrames;
  @Float()
  external double encoderLoad;
}

final class NativeWindowInfo extends Struct {
  @Int64()
  external int hwnd;
  external Pointer<Utf8> title;
}

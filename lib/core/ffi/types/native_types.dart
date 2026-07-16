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
  @Int64()
  external int monitorHandle;
  external Pointer<Utf8> encoderName;
  external Pointer<Utf8> micDeviceId;
  external Pointer<Utf8> sysAudioDeviceId;
  external Pointer<Utf8> webcamDeviceId;
  @Bool()
  external bool aiNoiseRemoval;
  @Bool()
  external bool aiAutoCaptions;
  @Bool()
  external bool aiSilenceDetection;
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

final class NativeAudioDeviceInfo extends Struct {
  external Pointer<Utf8> id;
  external Pointer<Utf8> name;
  @Bool()
  external bool isDefault;
}

final class NativeWebcamDeviceInfo extends Struct {
  external Pointer<Utf8> id;
  external Pointer<Utf8> name;
}

final class NativePluginInfo extends Struct {
  external Pointer<Utf8> name;
  external Pointer<Utf8> description;
  external Pointer<Utf8> author;
  external Pointer<Utf8> version;
}

final class NativeMonitorInfo extends Struct {
  @Int32()
  external int index;
  external Pointer<Utf8> name;
  @Int32()
  external int width;
  @Int32()
  external int height;
  @Bool()
  external bool isPrimary;
  @Int64()
  external int handle;
}

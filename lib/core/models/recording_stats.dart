class RecordingStats {
  final int elapsedMs;
  final int droppedFrames;
  final double encoderLoad;

  RecordingStats({
    required this.elapsedMs,
    required this.droppedFrames,
    required this.encoderLoad,
  });
}

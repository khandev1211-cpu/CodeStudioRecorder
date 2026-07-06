class RecordingConfig {
  final int width;
  final int height;
  final int fps;
  final String outputPath;
  final int? targetHwnd;
  final int? monitorHandle;

  RecordingConfig({
    this.width = 1920,
    this.height = 1080,
    this.fps = 60,
    required this.outputPath,
    this.targetHwnd,
    this.monitorHandle,
  });
}

class RecordingConfig {
  final int width;
  final int height;
  final int fps;
  final String outputPath;
  final int? targetHwnd;

  RecordingConfig({
    this.width = 1920,
    this.height = 1080,
    this.fps = 60,
    required this.outputPath,
    this.targetHwnd,
  });
}

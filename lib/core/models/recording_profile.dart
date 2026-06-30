class RecordingProfile {
  final String id;
  final String name;
  final int width;
  final int height;
  final int fps;
  final bool captureAudio;
  final bool captureCursor;
  final bool cursorHighlight;
  final bool clickAnimations;
  final bool smartZoom;
  final double zoomLevel;
  final String encoder; // e.g. "auto", "h264_nvenc", "libx264"
  final String? micDeviceId;
  final String? sysAudioDeviceId;
  final String? webcamDeviceId;

  RecordingProfile({
    required this.id,
    required this.name,
    this.width = 1920,
    this.height = 1080,
    this.fps = 60,
    this.captureAudio = true,
    this.captureCursor = true,
    this.cursorHighlight = true,
    this.clickAnimations = true,
    this.smartZoom = false,
    this.zoomLevel = 1.5,
    this.encoder = "auto",
    this.micDeviceId,
    this.sysAudioDeviceId,
    this.webcamDeviceId,
  });

  Map<String, dynamic> toJson() => {
    'id': id,
    'name': name,
    'width': width,
    'height': height,
    'fps': fps,
    'captureAudio': captureAudio,
    'captureCursor': captureCursor,
    'cursorHighlight': cursorHighlight,
    'clickAnimations': clickAnimations,
    'smartZoom': smartZoom,
    'zoomLevel': zoomLevel,
    'encoder': encoder,
    'micDeviceId': micDeviceId,
    'sysAudioDeviceId': sysAudioDeviceId,
    'webcamDeviceId': webcamDeviceId,
  };

  factory RecordingProfile.fromJson(Map<String, dynamic> json) => RecordingProfile(
    id: json['id'],
    name: json['name'],
    width: json['width'] ?? 1920,
    height: json['height'] ?? 1080,
    fps: json['fps'] ?? 60,
    captureAudio: json['captureAudio'] ?? true,
    captureCursor: json['captureCursor'] ?? true,
    cursorHighlight: json['cursorHighlight'] ?? true,
    clickAnimations: json['clickAnimations'] ?? true,
    smartZoom: json['smartZoom'] ?? false,
    zoomLevel: (json['zoomLevel'] ?? 1.5).toDouble(),
    encoder: json['encoder'] ?? "auto",
    micDeviceId: json['micDeviceId'],
    sysAudioDeviceId: json['sysAudioDeviceId'],
    webcamDeviceId: json['webcamDeviceId'],
  );

  static RecordingProfile defaultProfile() => RecordingProfile(
    id: 'default',
    name: 'Standard Recording',
  );
}

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
  final bool webcamEnabled;
  final String? webcamDeviceId;
  final double webcamX;
  final double webcamY;
  final double webcamWidth;
  final double webcamHeight;
  
  // AI Features
  final bool aiNoiseRemoval;
  final bool aiAutoCaptions;
  final bool aiSilenceDetection;

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
    this.webcamEnabled = false,
    this.webcamDeviceId,
    this.webcamX = 20,
    this.webcamY = 20,
    this.webcamWidth = 320,
    this.webcamHeight = 180,
    this.aiNoiseRemoval = false,
    this.aiAutoCaptions = false,
    this.aiSilenceDetection = false,
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
    'webcamEnabled': webcamEnabled,
    'webcamDeviceId': webcamDeviceId,
    'webcamX': webcamX,
    'webcamY': webcamY,
    'webcamWidth': webcamWidth,
    'webcamHeight': webcamHeight,
    'aiNoiseRemoval': aiNoiseRemoval,
    'aiAutoCaptions': aiAutoCaptions,
    'aiSilenceDetection': aiSilenceDetection,
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
    webcamEnabled: json['webcamEnabled'] ?? false,
    webcamDeviceId: json['webcamDeviceId'],
    webcamX: (json['webcamX'] ?? 20).toDouble(),
    webcamY: (json['webcamY'] ?? 20).toDouble(),
    webcamWidth: (json['webcamWidth'] ?? 320).toDouble(),
    webcamHeight: (json['webcamHeight'] ?? 180).toDouble(),
    aiNoiseRemoval: json['aiNoiseRemoval'] ?? false,
    aiAutoCaptions: json['aiAutoCaptions'] ?? false,
    aiSilenceDetection: json['aiSilenceDetection'] ?? false,
  );

  static RecordingProfile defaultProfile() => RecordingProfile(
    id: 'default',
    name: 'Standard Recording',
  );
}

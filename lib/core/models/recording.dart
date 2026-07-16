class Recording {
  final String id;
  final String title;
  final String filePath;
  final DateTime createdAt;
  final Duration duration;
  final int fileSize;
  final String? thumbnailPath;

  Recording({
    required this.id,
    required this.title,
    required this.filePath,
    required this.createdAt,
    required this.duration,
    required this.fileSize,
    this.thumbnailPath,
  });

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'title': title,
      'filePath': filePath,
      'createdAt': createdAt.toIso8601String(),
      'durationMs': duration.inMilliseconds,
      'fileSize': fileSize,
      'thumbnailPath': thumbnailPath,
    };
  }

  factory Recording.fromJson(Map<String, dynamic> json) {
    return Recording(
      id: json['id'],
      title: json['title'],
      filePath: json['filePath'],
      createdAt: DateTime.parse(json['createdAt']),
      duration: Duration(milliseconds: json['durationMs']),
      fileSize: json['fileSize'],
      thumbnailPath: json['thumbnailPath'],
    );
  }
}

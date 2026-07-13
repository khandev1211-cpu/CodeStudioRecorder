import 'dart:io';
import 'package:path/path.dart' as p;
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';

final setupServiceProvider = Provider((ref) => SetupService(ref.read(recordingServiceProvider)));

class SetupService {
  final RecordingService _service;
  SetupService(this._service);

  Future<bool> runIntegrityCheck() async {
    if (!_service.isInitialized) return false;
    return _service.checkSystemRequirements();
  }

  Future<void> initializeStorage() async {
    final userProfile = Platform.environment['USERPROFILE'] ?? '.';
    final dirs = [
      p.join(userProfile, 'Videos', 'CodeStudio'),
      p.join(userProfile, 'AppData', 'Local', 'CodeStudioRecorder'),
    ];

    for (final path in dirs) {
      final dir = Directory(path);
      if (!dir.existsSync()) {
        await dir.create(recursive: true);
      }
    }
  }

  bool checkSystemRequirements() {
    return _service.checkSystemRequirements();
  }
}

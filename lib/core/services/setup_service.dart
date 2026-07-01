import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';

final setupServiceProvider = Provider((ref) => SetupService(ref.read(recordingServiceProvider)));

class SetupService {
  final RecordingService _service;
  SetupService(this._service);

  Future<bool> runIntegrityCheck() async {
    if (!_service.isInitialized) return false;
    
    // Check system requirements (Win version, DX11, FFmpeg)
    bool sysOk = _service.checkSystemRequirements();
    if (!sysOk) return false;

    // Additional checks like folder permissions could go here
    return true;
  }

  bool checkSystemRequirements() {
    return _service.checkSystemRequirements();
  }
}

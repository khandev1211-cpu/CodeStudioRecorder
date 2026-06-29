import 'dart:async';
import 'dart:io';
import 'package:path/path.dart' as p;
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';
import 'package:codestudio_recorder/core/models/recording_stats.dart';
import 'package:codestudio_recorder/core/ffi/types/native_types.dart';
import 'package:codestudio_recorder/core/services/history_service.dart';
import 'package:codestudio_recorder/core/models/recording.dart';

class RecordingState {
  final RecordingStatus status;
  final Duration elapsedTime;
  final RecordingStats? stats;
  final String? lastError;

  RecordingState({
    this.status = RecordingStatus.idle,
    this.elapsedTime = Duration.zero,
    this.stats,
    this.lastError,
  });

  RecordingState copyWith({
    RecordingStatus? status,
    Duration? elapsedTime,
    RecordingStats? stats,
    String? lastError,
  }) {
    return RecordingState(
      status: status ?? this.status,
      elapsedTime: elapsedTime ?? this.elapsedTime,
      stats: stats ?? this.stats,
      lastError: lastError ?? this.lastError,
    );
  }
}

class RecordingNotifier extends StateNotifier<RecordingState> {
  final RecordingService _service;
  final HistoryService _historyService;
  Timer? _statsTimer;
  String? _lastOutputPath;

  RecordingNotifier(this._service, this._historyService) : super(RecordingState());

  Future<void> start(int width, int height, int fps, String? customPath, int targetHwnd) async {
    print("RecordingNotifier: Attempting to start recording...");
    state = state.copyWith(lastError: null);
    
    if (!_service.isInitialized) {
      print("RecordingNotifier: Service not initialized!");
      state = state.copyWith(lastError: "Native Engine failed to initialize.");
      return;
    }

    String outputPath = customPath ?? "";
    if (outputPath.isEmpty) {
      final userProfile = Platform.environment['USERPROFILE'] ?? '.';
      final timestamp = DateTime.now().millisecondsSinceEpoch;
      
      // Use a simpler fallback strategy
      final List<String> possibleBaseDirs = [
        p.join(userProfile, 'Videos'),
        p.join(userProfile, 'Documents'),
        userProfile,
        '.',
      ];

      String? activeDir;
      for (final base in possibleBaseDirs) {
        final dir = Directory(p.join(base, 'CodeStudio'));
        try {
          if (!dir.existsSync()) {
            dir.createSync(recursive: true);
          }
          if (dir.existsSync()) {
            activeDir = dir.path;
            break;
          }
        } catch (_) {
          continue;
        }
      }

      if (activeDir != null) {
        outputPath = p.join(activeDir, 'Rec_${timestamp}.mp4');
      } else {
        outputPath = p.join(userProfile, 'CodeStudio_Rec_${timestamp}.mp4');
      }
    }

    print("RecordingNotifier: Calling service.start with ${width}x${height}, $fps fps, path: $outputPath, hwnd: $targetHwnd");
    final result = _service.start(width, height, fps, outputPath, targetHwnd);
    print("RecordingNotifier: service.start returned: $result");
    
    if (result == 0) {
      _lastOutputPath = outputPath;
      state = state.copyWith(status: RecordingStatus.recording);
      _startStatsPolling();
    } else {
      state = state.copyWith(
        status: RecordingStatus.error,
        lastError: "Engine failed to start. Error code: $result",
      );
    }
  }

  Future<void> stop() async {
    final result = _service.stop();
    _statsTimer?.cancel();
    
    if (result == 0) {
      if (_lastOutputPath != null) {
        _historyService.saveRecording(Recording(
          id: DateTime.now().millisecondsSinceEpoch.toString(),
          title: p.basename(_lastOutputPath!),
          filePath: _lastOutputPath!,
          duration: state.elapsedTime,
          createdAt: DateTime.now(),
          fileSize: 0,
        ));
      }
      state = state.copyWith(status: RecordingStatus.completed);
    } else {
      state = state.copyWith(status: RecordingStatus.error, lastError: "Failed to stop properly.");
    }
  }

  void _startStatsPolling() {
    _statsTimer?.cancel();
    _statsTimer = Timer.periodic(const Duration(seconds: 1), (timer) {
      final stats = _service.getStats();
      final status = _service.status;

      state = state.copyWith(
        stats: stats,
        status: status,
        elapsedTime: Duration(milliseconds: stats.elapsedMs),
      );

      if (status == RecordingStatus.completed || status == RecordingStatus.error) {
        timer.cancel();
      }
    });
  }

  @override
  void dispose() {
    _statsTimer?.cancel();
    super.dispose();
  }
}

final recordingStateProvider = StateNotifierProvider<RecordingNotifier, RecordingState>((ref) {
  final service = ref.read(recordingServiceProvider);
  final history = ref.read(historyServiceProvider);
  return RecordingNotifier(service, history);
});

import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/ffi/types/native_types.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';
import 'package:codestudio_recorder/core/services/history_service.dart';
import 'package:codestudio_recorder/core/models/recording.dart';
import 'package:codestudio_recorder/core/models/recording_stats.dart';
import 'dart:async';
import 'package:intl/intl.dart';

class RecordingState {
  final RecordingStatus status;
  final Duration elapsedTime;
  final int droppedFrames;
  final double encoderLoad;

  RecordingState({
    required this.status,
    this.elapsedTime = Duration.zero,
    this.droppedFrames = 0,
    this.encoderLoad = 0.0,
  });

  RecordingState copyWith({
    RecordingStatus? status,
    Duration? elapsedTime,
    int? droppedFrames,
    double? encoderLoad,
  }) {
    return RecordingState(
      status: status ?? this.status,
      elapsedTime: elapsedTime ?? this.elapsedTime,
      droppedFrames: droppedFrames ?? this.droppedFrames,
      encoderLoad: encoderLoad ?? this.encoderLoad,
    );
  }
}

class RecordingNotifier extends StateNotifier<RecordingState> {
  final RecordingService _service;
  final HistoryService _historyService;
  Timer? _statsTimer;
  String? _lastOutputPath;

  RecordingNotifier(this._service, this._historyService) : super(RecordingState(status: RecordingStatus.idle));

  Future<void> start(int width, int height, int fps, String outputPath, int targetHwnd) async {
    final result = _service.start(width, height, fps, outputPath, targetHwnd);
    if (result == 0) {
      _lastOutputPath = outputPath;
      state = state.copyWith(status: RecordingStatus.recording);
      _startStatsPolling();
    }
  }

  Future<void> stop() async {
    final stats = _service.getStats();
    final duration = Duration(milliseconds: stats.elapsedMs);
    
    final result = _service.stop();
    if (result == 0) {
      if (_lastOutputPath != null) {
        await _historyService.saveRecording(Recording(
          id: DateTime.now().millisecondsSinceEpoch.toString(),
          title: "Recording ${DateFormat('yyyy-MM-dd HH:mm').format(DateTime.now())}",
          filePath: _lastOutputPath!,
          createdAt: DateTime.now(),
          duration: duration,
          fileSize: 0,
        ));
      }
      
      state = state.copyWith(status: RecordingStatus.completed);
      _stopStatsPolling();
    }
  }

  void _startStatsPolling() {
    _statsTimer?.cancel();
    _statsTimer = Timer.periodic(const Duration(milliseconds: 500), (timer) {
      final stats = _service.getStats();
      state = state.copyWith(
        status: _service.status,
        elapsedTime: Duration(milliseconds: stats.elapsedMs),
        droppedFrames: stats.droppedFrames,
        encoderLoad: stats.encoderLoad,
      );
    });
  }

  void _stopStatsPolling() {
    _statsTimer?.cancel();
    _statsTimer = null;
  }

  @override
  void dispose() {
    _stopStatsPolling();
    super.dispose();
  }
}

final recordingStateProvider = StateNotifierProvider<RecordingNotifier, RecordingState>((ref) {
  return RecordingNotifier(
    ref.watch(recordingServiceProvider),
    ref.watch(historyServiceProvider),
  );
});

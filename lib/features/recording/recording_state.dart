import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/ffi/types/native_types.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';
import 'dart:async';

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
  Timer? _statsTimer;

  RecordingNotifier(this._service) : super(RecordingState(status: RecordingStatus.idle));

  Future<void> start(int width, int height, int fps, String outputPath) async {
    final result = _service.start(width, height, fps, outputPath);
    if (result == 0) {
      state = state.copyWith(status: RecordingStatus.recording);
      _startStatsPolling();
    }
  }

  Future<void> stop() async {
    final result = _service.stop();
    if (result == 0) {
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
  return RecordingNotifier(ref.watch(recordingServiceProvider));
});

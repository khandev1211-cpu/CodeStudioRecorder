import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';

final monitorProvider = FutureProvider<List<({int index, String name, int width, int height, bool isPrimary, int handle})>>((ref) async {
  final service = ref.read(recordingServiceProvider);
  return service.getMonitors();
});

final selectedMonitorHandleProvider = StateProvider<int?>((ref) => null);

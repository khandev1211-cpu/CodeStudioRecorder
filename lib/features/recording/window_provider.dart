import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';

final windowsProvider = FutureProvider((ref) async {
  final service = ref.read(recordingServiceProvider);
  return service.getWindows();
});

final selectedWindowProvider = StateProvider<({int hwnd, String title})?>((ref) => null);

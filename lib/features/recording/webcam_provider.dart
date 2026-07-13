import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';

final webcamProvider = FutureProvider<List<({String id, String name})>>((ref) async {
  final service = ref.read(recordingServiceProvider);
  return service.getWebcams();
});

final selectedWebcamIdProvider = StateProvider<String?>((ref) => null);

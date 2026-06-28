import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';

class HomeScreen extends ConsumerWidget {
  const HomeScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final recordingService = ref.watch(recordingServiceProvider);

    return Scaffold(
      appBar: AppBar(
        title: const Text('CodeStudio Recorder'),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            const Icon(
              Icons.videocam,
              size: 100,
              color: Colors.red,
            ),
            const SizedBox(height: 20),
            ElevatedButton(
              onPressed: () {
                final result = recordingService.start(1920, 1080, 60, "test.mp4");
                ScaffoldMessenger.of(context).showSnackBar(
                  SnackBar(content: Text('Recording start result: $result')),
                );
              },
              child: const Text('Start Recording'),
            ),
            const SizedBox(height: 10),
            ElevatedButton(
              onPressed: () {
                final result = recordingService.stop();
                ScaffoldMessenger.of(context).showSnackBar(
                  SnackBar(content: Text('Recording stop result: $result')),
                );
              },
              child: const Text('Stop Recording'),
            ),
          ],
        ),
      ),
    );
  }
}

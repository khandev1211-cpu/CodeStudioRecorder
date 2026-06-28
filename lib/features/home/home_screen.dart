import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/features/recording/recording_state.dart';
import 'package:codestudio_recorder/core/ffi/types/native_types.dart';

class HomeScreen extends ConsumerWidget {
  const HomeScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final recordingState = ref.watch(recordingStateProvider);
    final recordingNotifier = ref.read(recordingStateProvider.notifier);

    final isRecording = recordingState.status == RecordingStatus.recording;

    return Scaffold(
      appBar: AppBar(
        title: const Text('CodeStudio Recorder'),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(
              Icons.videocam,
              size: 100,
              color: isRecording ? Colors.red : Colors.grey,
            ),
            if (isRecording) ...[
              const SizedBox(height: 10),
              Text(
                _formatDuration(recordingState.elapsedTime),
                style: const TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
              ),
              Text(
                'Status: ${recordingState.status.name}',
                style: const TextStyle(color: Colors.green),
              ),
            ],
            const SizedBox(height: 40),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                if (!isRecording)
                  ElevatedButton.icon(
                    onPressed: () => recordingNotifier.start(1920, 1080, 60, "recording.mp4"),
                    icon: const Icon(Icons.fiber_manual_record),
                    label: const Text('Start Recording'),
                    style: ElevatedButton.styleFrom(
                      backgroundColor: Colors.red.shade900,
                      foregroundColor: Colors.white,
                    ),
                  )
                else
                  ElevatedButton.icon(
                    onPressed: () => recordingNotifier.stop(),
                    icon: const Icon(Icons.stop),
                    label: const Text('Stop Recording'),
                  ),
              ],
            ),
            if (recordingState.status == RecordingStatus.completed)
              const Padding(
                padding: EdgeInsets.only(top: 20),
                child: Text('Recording saved successfully!'),
              ),
          ],
        ),
      ),
    );
  }

  String _formatDuration(Duration duration) {
    String twoDigits(int n) => n.toString().padLeft(2, "0");
    String twoDigitMinutes = twoDigits(duration.inMinutes.remainder(60));
    String twoDigitSeconds = twoDigits(duration.inSeconds.remainder(60));
    return "${twoDigits(duration.inHours)}:$twoDigitMinutes:$twoDigitSeconds";
  }
}

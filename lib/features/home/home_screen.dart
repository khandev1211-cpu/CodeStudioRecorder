import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';
import 'package:codestudio_recorder/features/recording/recording_state.dart';
import 'package:codestudio_recorder/features/recording/window_provider.dart';
import 'package:codestudio_recorder/core/ffi/types/native_types.dart';

class HomeScreen extends ConsumerWidget {
  const HomeScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final recordingState = ref.watch(recordingStateProvider);
    final recordingNotifier = ref.read(recordingStateProvider.notifier);
    final windows = ref.watch(windowsProvider);
    final selectedWindow = ref.watch(selectedWindowProvider);

    final isRecording = recordingState.status == RecordingStatus.recording;

    return Scaffold(
      appBar: AppBar(
        title: const Text('CodeStudio Recorder'),
        actions: [
          IconButton(
            icon: const Icon(Icons.history),
            onPressed: () => context.go('/history'),
          ),
          IconButton(
            icon: const Icon(Icons.settings),
            onPressed: () => context.go('/settings'),
          ),
          IconButton(
            icon: const Icon(Icons.refresh),
            onPressed: () => ref.refresh(windowsProvider),
          ),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(24.0),
        child: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(
                Icons.videocam,
                size: 80,
                color: isRecording ? Colors.red : Colors.grey,
              ),
              if (isRecording) ...[
                const SizedBox(height: 10),
                Text(
                  _formatDuration(recordingState.elapsedTime),
                  style: const TextStyle(fontSize: 32, fontWeight: FontWeight.bold, fontFamily: 'monospace'),
                ),
                Text(
                  'Recording ${selectedWindow?.title ?? "Full Screen"}',
                  style: const TextStyle(color: Colors.redAccent),
                ),
              ] else ...[
                const SizedBox(height: 20),
                const Text("Select target to record:"),
                const SizedBox(height: 10),
                Container(
                  width: 400,
                  padding: const EdgeInsets.symmetric(horizontal: 12),
                  decoration: BoxDecoration(
                    border: Border.all(color: Colors.white24),
                    borderRadius: BorderRadius.circular(8),
                  ),
                  child: windows.when(
                    data: (data) => DropdownButton<int>(
                      value: selectedWindow?.hwnd,
                      hint: const Text("Full Screen (Primary Monitor)"),
                      isExpanded: true,
                      underline: const SizedBox(),
                      items: [
                        const DropdownMenuItem<int>(
                          value: null,
                          child: Text("Full Screen"),
                        ),
                        ...data.map((win) => DropdownMenuItem<int>(
                              value: win.hwnd,
                              child: Text(win.title, overflow: TextOverflow.ellipsis),
                            )),
                      ],
                      onChanged: (val) {
                        if (val == null) {
                          ref.read(selectedWindowProvider.notifier).state = null;
                        } else {
                          final win = data.firstWhere((element) => element.hwnd == val);
                          ref.read(selectedWindowProvider.notifier).state = win;
                        }
                      },
                    ),
                    loading: () => const CircularProgressIndicator(),
                    error: (e, s) => Text("Error loading windows: $e"),
                  ),
                ),
              ],
              const SizedBox(height: 40),
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  if (!isRecording)
                    ElevatedButton.icon(
                      onPressed: () {
                        final hwnd = ref.read(selectedWindowProvider)?.hwnd ?? 0;
                        recordingNotifier.start(1920, 1080, 60, "recording.mp4", hwnd);
                      },
                      icon: const Icon(Icons.fiber_manual_record),
                      label: const Text('START RECORDING'),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: Colors.red.shade700,
                        foregroundColor: Colors.white,
                        padding: const EdgeInsets.symmetric(horizontal: 32, vertical: 16),
                        textStyle: const TextStyle(fontWeight: FontWeight.bold),
                      ),
                    )
                  else
                    ElevatedButton.icon(
                      onPressed: () => recordingNotifier.stop(),
                      icon: const Icon(Icons.stop),
                      label: const Text('STOP RECORDING'),
                      style: ElevatedButton.styleFrom(
                        padding: const EdgeInsets.symmetric(horizontal: 32, vertical: 16),
                      ),
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

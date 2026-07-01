import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';
import 'package:codestudio_recorder/features/recording/recording_state.dart';
import 'package:codestudio_recorder/features/recording/window_provider.dart';
import 'package:codestudio_recorder/core/ffi/types/native_types.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';
import 'package:codestudio_recorder/shared/theme/app_logo.dart';
import 'package:codestudio_recorder/features/recording/annotation_toolbar.dart';
import 'package:codestudio_recorder/features/recording/annotation_state.dart';

import 'package:codestudio_recorder/shared/widgets/volume_meter.dart';

import 'package:codestudio_recorder/core/models/recording_profile.dart';
import 'package:codestudio_recorder/core/services/profile_service.dart';

class HomeScreen extends ConsumerStatefulWidget {
  const HomeScreen({super.key});

  @override
  ConsumerState<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends ConsumerState<HomeScreen> {
  Offset? _startPoint;

  @override
  Widget build(BuildContext context) {
    final recordingState = ref.watch(recordingStateProvider);
    final recordingNotifier = ref.read(recordingStateProvider.notifier);
    final windows = ref.watch(windowsProvider);
    final selectedWindow = ref.watch(selectedWindowProvider);
    final service = ref.read(recordingServiceProvider);
    final annState = ref.watch(annotationProvider);
    final profileService = ref.watch(profileServiceProvider);
    final selectedProfile = ref.watch(selectedProfileProvider);

    final isRecording = recordingState.status == RecordingStatus.recording;

    // Listener for errors
    ref.listen(recordingStateProvider, (previous, next) {
      if (next.lastError != null) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text(next.lastError!),
            backgroundColor: Colors.red,
          ),
        );
      }
    });

    return Scaffold(
      appBar: AppBar(
        title: const AppLogo(size: 32, showText: true),
        actions: [
          IconButton(
            icon: const Icon(Icons.extension),
            tooltip: 'Plugins',
            onPressed: () => context.go('/plugins'),
          ),
          IconButton(
            icon: const Icon(Icons.description),
            tooltip: 'Recording Profiles',
            onPressed: () => context.go('/profiles'),
          ),
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
      body: Stack(
        children: [
          GestureDetector(
            onPanStart: (details) {
              if (isRecording && annState.tool != AnnotationTool.none) {
                setState(() => _startPoint = details.localPosition);
              }
            },
            onPanEnd: (details) {
              setState(() => _startPoint = null);
            },
            onPanUpdate: (details) {
              if (isRecording && _startPoint != null && annState.tool != AnnotationTool.none) {
                final RenderBox renderBox = context.findRenderObject() as RenderBox;
                final size = renderBox.size;
                
                // Scale coordinates from logical pixels to recording resolution
                double scaleX = selectedProfile.width / size.width;
                double scaleY = selectedProfile.height / size.height;

                service.undoAnnotation(); // Remove previous preview
                
                int typeIndex = 0;
                switch(annState.tool) {
                  case AnnotationTool.line: typeIndex = 0; break;
                  case AnnotationTool.rect: typeIndex = 1; break;
                  case AnnotationTool.ellipse: typeIndex = 2; break;
                  case AnnotationTool.arrow: typeIndex = 3; break;
                  default: break;
                }

                service.addAnnotation(
                  typeIndex, 
                  _startPoint!.dx * scaleX, _startPoint!.dy * scaleY, 
                  details.localPosition.dx * scaleX, details.localPosition.dy * scaleY, 
                  annState.color.value, 
                  annState.strokeWidth
                );
              }
            },
            onTapDown: (details) {
              // Click reporting is now handled by the global native hook
              // for better reliability across all applications.
            },
            child: Container(
              color: Colors.transparent, // Capture gestures
              padding: const EdgeInsets.all(24.0),
              child: Center(
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    if (!isRecording)
                      const AppLogo(size: 120)
                    else
                      const Icon(
                        Icons.videocam,
                        size: 80,
                        color: Colors.red,
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
                      const SizedBox(height: 20),
                      SizedBox(
                        width: 250,
                        child: Column(
                          children: [
                            VolumeMeter(
                              level: recordingState.micLevel,
                              label: "MICROPHONE",
                              color: Colors.blueAccent,
                            ),
                            const SizedBox(height: 12),
                            VolumeMeter(
                              level: recordingState.systemLevel,
                              label: "SYSTEM AUDIO",
                            ),
                          ],
                        ),
                      ),
                      if (recordingState.currentCaption != null) ...[
                        const SizedBox(height: 30),
                        Container(
                          padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
                          decoration: BoxDecoration(
                            color: Colors.black54,
                            borderRadius: BorderRadius.circular(8),
                          ),
                          child: Text(
                            recordingState.currentCaption!,
                            style: const TextStyle(
                              color: Colors.yellowAccent,
                              fontSize: 18,
                              fontStyle: FontStyle.italic,
                            ),
                            textAlign: TextAlign.center,
                          ),
                        ),
                      ],
                    ] else ...[
                      const SizedBox(height: 20),
                      const Text("Recording Profile:"),
                      const SizedBox(height: 5),
                      Container(
                        width: 400,
                        padding: const EdgeInsets.symmetric(horizontal: 12),
                        decoration: BoxDecoration(
                          border: Border.all(color: Colors.white24),
                          borderRadius: BorderRadius.circular(8),
                        ),
                        child: DropdownButton<String>(
                          value: selectedProfile.id,
                          isExpanded: true,
                          underline: const SizedBox(),
                          items: profileService.profiles.map((p) => DropdownMenuItem(
                            value: p.id,
                            child: Text(p.name),
                          )).toList(),
                          onChanged: (val) {
                            if (val != null) {
                              final profile = profileService.profiles.firstWhere((p) => p.id == val);
                              ref.read(selectedProfileProvider.notifier).state = profile;
                            }
                          },
                        ),
                      ),
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
                              
                              // Apply profile settings before starting
                              service.setProcessorEnabled(0, selectedProfile.cursorHighlight);
                              service.setProcessorEnabled(1, selectedProfile.clickAnimations);
                              service.setProcessorEnabled(3, selectedProfile.smartZoom);
                              if (selectedProfile.smartZoom) {
                                service.setZoomLevel(selectedProfile.zoomLevel);
                              }

                              recordingNotifier.start(
                                selectedProfile.width, 
                                selectedProfile.height, 
                                selectedProfile.fps, 
                                null, 
                                hwnd,
                                selectedProfile.encoder,
                                micId: selectedProfile.micDeviceId,
                                sysId: selectedProfile.sysAudioDeviceId,
                                webcamId: selectedProfile.webcamDeviceId,
                                aiNoise: selectedProfile.aiNoiseRemoval,
                                aiCaptions: selectedProfile.aiAutoCaptions,
                                aiSilence: selectedProfile.aiSilenceDetection,
                              );
                            },
                            icon: const Icon(Icons.fiber_manual_record),
                            label: const Text('START RECORDING'),
                            style: ElevatedButton.styleFrom(
                              backgroundColor: Colors.red.shade700,
                              foregroundColor: Colors.white,
                              padding: const EdgeInsets.symmetric(horizontal: 32, vertical: 16),
                            ),
                          )
                        else
                          Row(
                            mainAxisAlignment: MainAxisAlignment.center,
                            children: [
                              ElevatedButton.icon(
                                onPressed: () => _showAddMarkerDialog(context, service),
                                icon: const Icon(Icons.bookmark_add),
                                label: const Text('MARK'),
                                style: ElevatedButton.styleFrom(
                                  backgroundColor: Colors.white10,
                                  padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 16),
                                ),
                              ),
                              const SizedBox(width: 12),
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
          ),
          if (isRecording)
            Positioned(
              bottom: 20,
              left: 0,
              right: 0,
              child: Center(
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    if (annState.tool == AnnotationTool.none)
                      ElevatedButton.icon(
                        onPressed: () => ref.read(annotationProvider.notifier).setTool(AnnotationTool.line),
                        icon: const Icon(Icons.edit),
                        label: const Text('DRAW ON SCREEN'),
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.white10,
                        ),
                      )
                    else
                      const AnnotationToolbar(),
                  ],
                ),
              ),
            ),
        ],
      ),
    );
  }

  String _formatDuration(Duration duration) {
    String twoDigits(int n) => n.toString().padLeft(2, "0");
    String twoDigitMinutes = twoDigits(duration.inMinutes.remainder(60));
    String twoDigitSeconds = twoDigits(duration.inSeconds.remainder(60));
    return "${twoDigits(duration.inHours)}:$twoDigitMinutes:$twoDigitSeconds";
  }

  void _showAddMarkerDialog(BuildContext context, RecordingService service) {
    final controller = TextEditingController();
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Add Chapter Marker'),
        content: TextField(
          controller: controller,
          autofocus: true,
          decoration: const InputDecoration(hintText: 'e.g. Setting up UI'),
          onSubmitted: (_) {
            service.addChapterMarker(controller.text);
            Navigator.pop(context);
          },
        ),
        actions: [
          TextButton(onPressed: () => Navigator.pop(context), child: const Text('CANCEL')),
          ElevatedButton(
            onPressed: () {
              service.addChapterMarker(controller.text);
              Navigator.pop(context);
            },
            child: const Text('ADD'),
          ),
        ],
      ),
    );
  }
}

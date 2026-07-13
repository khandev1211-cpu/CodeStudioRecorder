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
import 'package:codestudio_recorder/features/recording/monitor_provider.dart';
import 'package:codestudio_recorder/features/home/widgets/recording_dashboard.dart';
import 'package:codestudio_recorder/features/export/export_dialog.dart';
import 'package:codestudio_recorder/core/services/history_service.dart';

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
    final monitors = ref.watch(monitorProvider);
    final selectedMonitorHandle = ref.watch(selectedMonitorHandleProvider);

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
      
      if (next.status == RecordingStatus.completed && previous?.status != RecordingStatus.completed) {
        final lastRecording = ref.read(historyServiceProvider).history.firstOrNull;
        if (lastRecording != null) {
          showDialog(
            context: context,
            barrierDismissible: false,
            builder: (context) => ExportDialog(recording: lastRecording),
          );
        }
      }
    });

    return Scaffold(
      appBar: AppBar(
        title: const AppLogo(size: 28, showText: true),
        backgroundColor: Colors.transparent,
        elevation: 0,
        actions: [
          IconButton(
            icon: const Icon(Icons.extension_outlined),
            tooltip: 'Plugins',
            onPressed: () => context.go('/plugins'),
          ),
          IconButton(
            icon: const Icon(Icons.description_outlined),
            tooltip: 'Profiles',
            onPressed: () => context.go('/profiles'),
          ),
          IconButton(
            icon: const Icon(Icons.history_outlined),
            tooltip: 'History',
            onPressed: () => context.go('/history'),
          ),
          IconButton(
            icon: const Icon(Icons.settings_outlined),
            tooltip: 'Settings',
            onPressed: () => context.go('/settings'),
          ),
          const SizedBox(width: 8),
        ],
      ),
      body: Row(
        children: [
          // Left Side - Controls & Dashboard
          Container(
            width: 450,
            padding: const EdgeInsets.all(24.0),
            decoration: const BoxDecoration(
              border: Border(right: BorderSide(color: Colors.white10)),
            ),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                if (!isRecording) ...[
                  const RecordingDashboard(),
                  const Spacer(),
                  _buildStartButton(context, recordingNotifier, selectedProfile, selectedWindow, selectedMonitor, service),
                ] else ...[
                  _buildRecordingActiveView(recordingState, selectedWindow, recordingNotifier, context, service),
                ],
              ],
            ),
          ),
          
          // Right Side - Visual Preview / Status
          Expanded(
            child: Container(
              color: const Color(0xFF0A0A0A),
              child: Center(
                child: isRecording 
                  ? _buildLiveStats(recordingState)
                  : _buildIdlePreview(selectedProfile),
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildStartButton(BuildContext context, RecordingNotifier notifier, RecordingProfile profile, WindowInfo? window, int? monitor, RecordingService service) {
    return SizedBox(
      width: double.infinity,
      height: 60,
      child: ElevatedButton.icon(
        onPressed: () {
          service.setProcessorEnabled(0, profile.cursorHighlight);
          service.setProcessorEnabled(1, profile.clickAnimations);
          service.setProcessorEnabled(3, profile.smartZoom);
          if (profile.smartZoom) service.setZoomLevel(profile.zoomLevel);
          service.setProcessorEnabled(4, profile.webcamEnabled);
          if (profile.webcamEnabled) {
            service.setWebcamPosition(profile.webcamX, profile.webcamY, profile.webcamWidth, profile.webcamHeight);
          }

          notifier.start(
            profile.width, profile.height, profile.fps, null, 
            window?.hwnd ?? 0, profile.encoder,
            monitorHandle: monitor,
            micId: profile.micDeviceId,
            sysId: profile.sysAudioDeviceId,
            webcamId: profile.webcamDeviceId,
            aiNoise: profile.aiNoiseRemoval,
            aiCaptions: profile.aiAutoCaptions,
            aiSilence: profile.aiSilenceDetection,
          );
        },
        icon: const Icon(Icons.fiber_manual_record, size: 28),
        label: const Text('START RECORDING', style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold, letterSpacing: 1.1)),
        style: ElevatedButton.styleFrom(
          backgroundColor: const Color(0xFFFF3B3B),
          foregroundColor: Colors.white,
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
        ),
      ),
    );
  }

  Widget _buildRecordingActiveView(RecordingState state, WindowInfo? window, RecordingNotifier notifier, BuildContext context, RecordingService service) {
    return Column(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        const Icon(Icons.videocam, size: 80, color: Color(0xFFFF3B3B)),
        const SizedBox(height: 16),
        Text(
          _formatDuration(state.elapsedTime),
          style: const TextStyle(fontSize: 48, fontWeight: FontWeight.bold, fontFamily: 'monospace'),
        ),
        Text(
          'RECORDING ACTIVE',
          style: TextStyle(color: Colors.white.withOpacity(0.5), letterSpacing: 2, fontSize: 12),
        ),
        const SizedBox(height: 40),
        VolumeMeter(level: state.micLevel, label: "MIC"),
        const SizedBox(height: 16),
        VolumeMeter(level: state.systemLevel, label: "SYSTEM"),
        const Spacer(),
        Row(
          children: [
            Expanded(
              child: OutlinedButton.icon(
                onPressed: () => _showAddMarkerDialog(context, service),
                icon: const Icon(Icons.bookmark_add_outlined),
                label: const Text("MARK"),
                style: OutlinedButton.styleFrom(padding: const EdgeInsets.symmetric(vertical: 20)),
              ),
            ),
            const SizedBox(width: 12),
            Expanded(
              child: ElevatedButton.icon(
                onPressed: () => notifier.stop(),
                icon: const Icon(Icons.stop),
                label: const Text("STOP"),
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.white,
                  foregroundColor: Colors.black,
                  padding: const EdgeInsets.symmetric(vertical: 20),
                ),
              ),
            ),
          ],
        ),
      ],
    );
  }

  Widget _buildIdlePreview(RecordingProfile profile) {
    return Column(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        // Visual Layout Mockup
        Container(
          width: 480,
          height: 270,
          decoration: BoxDecoration(
            color: Colors.black,
            borderRadius: BorderRadius.circular(16),
            border: Border.all(color: Colors.white10, width: 2),
            boxShadow: [
              BoxShadow(color: Colors.black.withOpacity(0.5), blurRadius: 40, spreadRadius: 10),
            ],
          ),
          child: Stack(
            children: [
              Center(
                child: Opacity(
                  opacity: 0.1,
                  child: Image.asset("assets/images/grid.png", fit: BoxFit.cover, errorBuilder: (_, __, ___) => const Icon(Icons.grid_3x3, size: 100)),
                ),
              ),
              if (profile.webcamEnabled)
                Positioned(
                  left: profile.webcamX / 4, // Scale to mockup size
                  top: profile.webcamY / 4,
                  child: Container(
                    width: profile.webcamWidth / 4,
                    height: profile.webcamHeight / 4,
                    decoration: BoxDecoration(
                      color: const Color(0xFFFF3B3B).withOpacity(0.2),
                      borderRadius: BorderRadius.circular(8),
                      border: Border.all(color: const Color(0xFFFF3B3B), width: 2),
                    ),
                    child: const Center(
                      child: Icon(Icons.videocam, color: Color(0xFFFF3B3B)),
                    ),
                  ),
                ),
              const Positioned(
                bottom: 12,
                left: 12,
                child: Row(
                  children: [
                    Icon(Icons.fiber_manual_record, color: Colors.red, size: 12),
                    SizedBox(width: 8),
                    Text("LIVE PREVIEW MODE", style: TextStyle(fontSize: 10, fontWeight: FontWeight.bold, color: Colors.white54)),
                  ],
                ),
              ),
            ],
          ),
        ),
        const SizedBox(height: 48),
        const AppLogo(size: 80),
        const SizedBox(height: 16),
        Text(
          profile.name.toUpperCase(),
          style: const TextStyle(fontSize: 12, letterSpacing: 4, fontWeight: FontWeight.w400, color: Colors.white54),
        ),
        const SizedBox(height: 8),
        Text(
          "${profile.width} × ${profile.height} @ ${profile.fps} FPS",
          style: const TextStyle(color: Colors.white12, fontSize: 14),
        ),
      ],
    );
  }

  Widget _buildLiveStats(RecordingState state) {
    return Column(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        if (state.currentCaption != null)
          Container(
            padding: const EdgeInsets.all(20),
            margin: const EdgeInsets.symmetric(horizontal: 40),
            decoration: BoxDecoration(
              color: Colors.white.withOpacity(0.05),
              borderRadius: BorderRadius.circular(16),
            ),
            child: Text(
              state.currentCaption!,
              style: const TextStyle(fontSize: 24, fontStyle: FontStyle.italic, color: Colors.yellowAccent),
              textAlign: TextAlign.center,
            ),
          ),
      ],
    );
  }
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

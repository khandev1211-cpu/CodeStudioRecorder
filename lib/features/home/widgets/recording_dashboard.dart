import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/models/recording_profile.dart';
import 'package:codestudio_recorder/features/recording/recording_state.dart';
import 'package:codestudio_recorder/features/recording/window_provider.dart';
import 'package:codestudio_recorder/features/recording/monitor_provider.dart';
import 'package:codestudio_recorder/features/recording/webcam_provider.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';
import 'package:codestudio_recorder/core/services/profile_service.dart';

class RecordingDashboard extends ConsumerWidget {
  const RecordingDashboard({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final profile = ref.watch(selectedProfileProvider);
    final profileService = ref.watch(profileServiceProvider);
    final selectedWindow = ref.watch(selectedWindowProvider);
    final selectedMonitor = ref.watch(selectedMonitorHandleProvider);
    final windows = ref.watch(windowsProvider);
    final monitors = ref.watch(monitorProvider);
    final webcams = ref.watch(webcamProvider);

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text(
          "RECORDING SETUP",
          style: TextStyle(
            fontSize: 12,
            fontWeight: FontWeight.bold,
            letterSpacing: 1.2,
            color: Colors.white54,
          ),
        ),
        const SizedBox(height: 16),

        // Profile Selector
        _buildCard(
          child: _buildSettingRow(
            icon: Icons.style,
            label: "Profile",
            trailing: DropdownButton<String>(
              value: profile.id,
              underline: const SizedBox(),
              items: profileService.profiles.map((p) => DropdownMenuItem(
                value: p.id,
                child: Text(p.name),
              )).toList(),
              onChanged: (val) {
                if (val != null) {
                  final p = profileService.profiles.firstWhere((element) => element.id == val);
                  ref.read(selectedProfileProvider.notifier).state = p;
                }
              },
            ),
          ),
        ),
        
        const SizedBox(height: 12),
        
        // Target Selector
        _buildCard(
          child: Column(
            children: [
              _buildSettingRow(
                icon: Icons.monitor,
                label: "Target Source",
                trailing: windows.when(
                  data: (data) => DropdownButton<int?>(
                    value: selectedWindow?.hwnd,
                    underline: const SizedBox(),
                    items: [
                      const DropdownMenuItem(value: null, child: Text("Full Screen")),
                      ...data.map((win) => DropdownMenuItem(
                        value: win.hwnd,
                        child: SizedBox(
                          width: 200,
                          child: Text(win.title, overflow: TextOverflow.ellipsis),
                        ),
                      )),
                    ],
                    onChanged: (val) {
                      if (val == null) {
                        ref.read(selectedWindowProvider.notifier).state = null;
                      } else {
                        ref.read(selectedWindowProvider.notifier).state = 
                          data.firstWhere((w) => w.hwnd == val);
                      }
                    },
                  ),
                  loading: () => const CircularProgressIndicator(strokeWidth: 2),
                  error: (_, __) => const Text("Error"),
                ),
              ),
              if (selectedWindow == null) ...[
                const Divider(height: 1, color: Colors.white10),
                _buildSettingRow(
                  icon: Icons.screenshot_monitor,
                  label: "Select Display",
                  trailing: monitors.when(
                    data: (data) => DropdownButton<int?>(
                      value: selectedMonitor,
                      underline: const SizedBox(),
                      items: [
                        const DropdownMenuItem(value: null, child: Text("Primary Display")),
                        ...data.map((mon) => DropdownMenuItem(
                          value: mon.handle,
                          child: Text(mon.name),
                        )),
                      ],
                      onChanged: (val) => ref.read(selectedMonitorHandleProvider.notifier).state = val,
                    ),
                    loading: () => const CircularProgressIndicator(strokeWidth: 2),
                    error: (_, __) => const Text("Error"),
                  ),
                ),
              ],
            ],
          ),
        ),

        const SizedBox(height: 12),

        // Webcam Selector
        _buildCard(
          child: Column(
            children: [
              _buildSettingRow(
                icon: Icons.videocam,
                label: "Webcam PiP",
                trailing: Switch(
                  value: profile.webcamEnabled,
                  onChanged: (val) {
                    profileService.saveProfile(profile.copyWith(webcamEnabled: val));
                    ref.read(selectedProfileProvider.notifier).state = profile.copyWith(webcamEnabled: val);
                  },
                ),
              ),
              if (profile.webcamEnabled) ...[
                const Divider(height: 1, color: Colors.white10),
                _buildSettingRow(
                  icon: Icons.camera_alt,
                  label: "Camera",
                  trailing: webcams.when(
                    data: (data) => DropdownButton<String?>(
                      value: profile.webcamDeviceId,
                      hint: const Text("None"),
                      underline: const SizedBox(),
                      items: [
                        const DropdownMenuItem(value: null, child: Text("No Camera")),
                        ...data.map((cam) => DropdownMenuItem(
                          value: cam.id,
                          child: SizedBox(width: 150, child: Text(cam.name, overflow: TextOverflow.ellipsis)),
                        )),
                      ],
                      onChanged: (val) {
                         profileService.saveProfile(profile.copyWith(webcamDeviceId: val));
                         ref.read(selectedProfileProvider.notifier).state = profile.copyWith(webcamDeviceId: val);
                      },
                    ),
                    loading: () => const CircularProgressIndicator(strokeWidth: 2),
                    error: (_, __) => const Text("Error"),
                  ),
                ),
              ],
            ],
          ),
        ),

        const SizedBox(height: 12),

        // Audio Selector
        _buildCard(
          child: Column(
            children: [
              _buildSettingRow(
                icon: profile.captureAudio ? Icons.mic : Icons.mic_off,
                label: "Audio Input",
                trailing: Text(profile.micDeviceId == null ? "Default" : "Custom", style: const TextStyle(color: Colors.white38)),
              ),
              const Divider(height: 1, color: Colors.white10),
              _buildSettingRow(
                icon: Icons.auto_awesome,
                label: "AI Noise Removal",
                trailing: Switch(
                  value: profile.aiNoiseRemoval,
                  onChanged: (val) {
                    profileService.saveProfile(profile.copyWith(aiNoiseRemoval: val));
                    ref.read(selectedProfileProvider.notifier).state = profile.copyWith(aiNoiseRemoval: val);
                  },
                ),
              ),
            ],
          ),
        ),
      ],
    );
  }
  }

  Widget _buildCard({required Widget child}) {
    return Container(
      decoration: BoxDecoration(
        color: const Color(0xFF1E1E1E),
        borderRadius: BorderRadius.circular(12),
        border: Border.all(color: Colors.white10),
      ),
      child: child,
    );
  }

  Widget _buildSettingRow({required IconData icon, required String label, required Widget trailing}) {
    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
      child: Row(
        children: [
          Icon(icon, size: 20, color: Colors.white70),
          const SizedBox(width: 16),
          Text(label, style: const TextStyle(fontSize: 14, fontWeight: FontWeight.w500)),
          const Spacer(),
          trailing,
        ],
      ),
    );
  }
}

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/models/recording_profile.dart';
import 'package:codestudio_recorder/core/services/profile_service.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';
import 'package:go_router/go_router.dart';

class ProfileDetailsScreen extends ConsumerStatefulWidget {
  final String profileId;
  const ProfileDetailsScreen({super.key, required this.profileId});

  @override
  ConsumerState<ProfileDetailsScreen> createState() => _ProfileDetailsScreenState();
}

class _ProfileDetailsScreenState extends ConsumerState<ProfileDetailsScreen> {
  late RecordingProfile _profile;
  List<({String id, String name, bool isDefault})> _mics = [];
  List<({String id, String name, bool isDefault})> _speakers = [];
  List<({String id, String name})> _webcams = [];

  @override
  void initState() {
    super.initState();
    final service = ref.read(profileServiceProvider);
    _profile = service.profiles.firstWhere((p) => p.id == widget.profileId);
    _loadDevices();
  }

  void _loadDevices() {
    final service = ref.read(recordingServiceProvider);
    setState(() {
      _mics = service.getAudioDevices(true);
      _speakers = service.getAudioDevices(false);
      _webcams = service.getWebcams();
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Edit Profile: ${_profile.name}'),
        actions: [
          IconButton(
            icon: const Icon(Icons.save),
            onPressed: () async {
              await ref.read(profileServiceProvider).saveProfile(_profile);
              if (mounted) context.pop();
            },
          ),
        ],
      ),
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          const Text("Video Settings", style: TextStyle(fontWeight: FontWeight.bold)),
          const SizedBox(height: 10),
          Row(
            children: [
              Expanded(
                child: TextField(
                  decoration: const InputDecoration(labelText: "Width"),
                  keyboardType: TextInputType.number,
                  controller: TextEditingController(text: _profile.width.toString()),
                  onChanged: (v) => _profile = _profile.copyWith(width: int.tryParse(v) ?? 1920),
                ),
              ),
              const SizedBox(width: 16),
              Expanded(
                child: TextField(
                  decoration: const InputDecoration(labelText: "Height"),
                  keyboardType: TextInputType.number,
                  controller: TextEditingController(text: _profile.height.toString()),
                  onChanged: (v) => _profile = _profile.copyWith(height: int.tryParse(v) ?? 1080),
                ),
              ),
            ],
          ),
          const SizedBox(height: 16),
          DropdownButtonFormField<int>(
            decoration: const InputDecoration(labelText: "FPS"),
            value: _profile.fps,
            items: [30, 60, 120].map((f) => DropdownMenuItem(value: f, child: Text("$f FPS"))).toList(),
            onChanged: (v) => setState(() => _profile = _profile.copyWith(fps: v!)),
          ),
          const SizedBox(height: 16),
          DropdownButtonFormField<String>(
            decoration: const InputDecoration(labelText: "Hardware Encoder"),
            value: _profile.encoder,
            items: const [
              DropdownMenuItem(value: "auto", child: Text("Auto-Detect (Recommended)")),
              DropdownMenuItem(value: "h264_nvenc", child: Text("NVIDIA NVENC")),
              DropdownMenuItem(value: "h264_amf", child: Text("AMD AMF")),
              DropdownMenuItem(value: "h264_qsv", child: Text("Intel QuickSync")),
              DropdownMenuItem(value: "libx264", child: Text("Software (CPU)")),
            ],
            onChanged: (v) => setState(() => _profile = _profile.copyWith(encoder: v!)),
          ),
          const Divider(height: 40),
          const Text("Audio Settings", style: TextStyle(fontWeight: FontWeight.bold)),
          const SizedBox(height: 10),
          DropdownButtonFormField<String?>(
            decoration: const InputDecoration(labelText: "Microphone"),
            value: _profile.micDeviceId,
            items: [
              const DropdownMenuItem(value: null, child: Text("Default Communication Device")),
              ..._mics.map((m) => DropdownMenuItem(value: m.id, child: Text(m.name, overflow: TextOverflow.ellipsis))),
            ],
            onChanged: (v) => setState(() => _profile = _profile.copyWith(micDeviceId: v)),
          ),
          const SizedBox(height: 16),
          DropdownButtonFormField<String?>(
            decoration: const InputDecoration(labelText: "System Audio (Loopback)"),
            value: _profile.sysAudioDeviceId,
            items: [
              const DropdownMenuItem(value: null, child: Text("Default Playback Device")),
              ..._speakers.map((s) => DropdownMenuItem(value: s.id, child: Text(s.name, overflow: TextOverflow.ellipsis))),
            ],
            onChanged: (v) => setState(() => _profile = _profile.copyWith(sysAudioDeviceId: v)),
          ),
          const SizedBox(height: 16),
          DropdownButtonFormField<String?>(
            decoration: const InputDecoration(labelText: "Webcam Device"),
            value: _profile.webcamDeviceId,
            items: [
              const DropdownMenuItem(value: null, child: Text("None")),
              ..._webcams.map((c) => DropdownMenuItem(value: c.id, child: Text(c.name, overflow: TextOverflow.ellipsis))),
            ],
            onChanged: (v) => setState(() => _profile = _profile.copyWith(webcamDeviceId: v)),
          ),
          if (_profile.webcamDeviceId != null) ...[
            SwitchListTile(
              title: const Text("Enable Webcam Overlay (PiP)"),
              value: _profile.webcamEnabled,
              onChanged: (v) => setState(() => _profile = _profile.copyWith(webcamEnabled: v)),
            ),
            if (_profile.webcamEnabled) ...[
              Padding(
                padding: const EdgeInsets.symmetric(horizontal: 16),
                child: Column(
                  children: [
                    Row(
                      children: [
                        Expanded(
                          child: TextField(
                            decoration: const InputDecoration(labelText: "X Position"),
                            keyboardType: TextInputType.number,
                            controller: TextEditingController(text: _profile.webcamX.toInt().toString()),
                            onChanged: (v) => _profile = _profile.copyWith(webcamX: double.tryParse(v) ?? 20),
                          ),
                        ),
                        const SizedBox(width: 16),
                        Expanded(
                          child: TextField(
                            decoration: const InputDecoration(labelText: "Y Position"),
                            keyboardType: TextInputType.number,
                            controller: TextEditingController(text: _profile.webcamY.toInt().toString()),
                            onChanged: (v) => _profile = _profile.copyWith(webcamY: double.tryParse(v) ?? 20),
                          ),
                        ),
                      ],
                    ),
                    const SizedBox(height: 16),
                    Row(
                      children: [
                        Expanded(
                          child: TextField(
                            decoration: const InputDecoration(labelText: "Width"),
                            keyboardType: TextInputType.number,
                            controller: TextEditingController(text: _profile.webcamWidth.toInt().toString()),
                            onChanged: (v) => _profile = _profile.copyWith(webcamWidth: double.tryParse(v) ?? 320),
                          ),
                        ),
                        const SizedBox(width: 16),
                        Expanded(
                          child: TextField(
                            decoration: const InputDecoration(labelText: "Height"),
                            keyboardType: TextInputType.number,
                            controller: TextEditingController(text: _profile.webcamHeight.toInt().toString()),
                            onChanged: (v) => _profile = _profile.copyWith(webcamHeight: double.tryParse(v) ?? 180),
                          ),
                        ),
                      ],
                    ),
                  ],
                ),
              ),
            ],
          ],
          const Divider(height: 40),
          const Text("Effects (Default State)", style: TextStyle(fontWeight: FontWeight.bold)),
          SwitchListTile(
            title: const Text("Cursor Highlight"),
            value: _profile.cursorHighlight,
            onChanged: (v) => setState(() => _profile = _profile.copyWith(cursorHighlight: v)),
          ),
          SwitchListTile(
            title: const Text("Click Animations"),
            value: _profile.clickAnimations,
            onChanged: (v) => setState(() => _profile = _profile.copyWith(clickAnimations: v)),
          ),
          SwitchListTile(
            title: const Text("Smart Zoom"),
            value: _profile.smartZoom,
            onChanged: (v) => setState(() => _profile = _profile.copyWith(smartZoom: v)),
          ),
          const Divider(height: 40),
          const Text("AI Features (Phase 4)", style: TextStyle(fontWeight: FontWeight.bold)),
          SwitchListTile(
            title: const Text("AI Background Noise Removal"),
            subtitle: const Text("Uses DeepFilterNet to clean microphone audio"),
            value: _profile.aiNoiseRemoval,
            onChanged: (v) => setState(() => _profile = _profile.copyWith(aiNoiseRemoval: v)),
          ),
          SwitchListTile(
            title: const Text("AI Auto-Captions (Local Whisper)"),
            subtitle: const Text("Generate real-time captions locally"),
            value: _profile.aiAutoCaptions,
            onChanged: (v) => setState(() => _profile = _profile.copyWith(aiAutoCaptions: v)),
          ),
          SwitchListTile(
            title: const Text("AI Silence Detection"),
            subtitle: const Text("Automatically mark silent segments in history"),
            value: _profile.aiSilenceDetection,
            onChanged: (v) => setState(() => _profile = _profile.copyWith(aiSilenceDetection: v)),
          ),
          const Divider(height: 40),
          const Text("Advanced Audio Controls", style: TextStyle(fontWeight: FontWeight.bold)),
          const SizedBox(height: 16),
          ListTile(
            title: const Text("Noise Suppression Strength"),
            subtitle: const Text("Adjust how aggressively to filter background noise"),
            trailing: SizedBox(
              width: 200,
              child: Slider(
                value: 0.5, // Mock value for now
                onChanged: (v) {},
                activeColor: Colors.blueAccent,
              ),
            ),
          ),
          CheckboxListTile(
            title: const Text("High-Pass Filter"),
            subtitle: const Text("Remove low-frequency hum from microphone"),
            value: true,
            onChanged: (v) {},
          ),
          CheckboxListTile(
            title: const Text("Automatic Gain Control"),
            subtitle: const Text("Keep audio levels consistent throughout recording"),
            value: true,
            onChanged: (v) {},
          ),
        ],
      ),
    );
  }
}

extension on RecordingProfile {
  RecordingProfile copyWith({
    int? width,
    int? height,
    int? fps,
    String? encoder,
    String? micDeviceId,
    String? sysAudioDeviceId,
    bool? webcamEnabled,
    String? webcamDeviceId,
    double? webcamX,
    double? webcamY,
    double? webcamWidth,
    double? webcamHeight,
    bool? cursorHighlight,
    bool? clickAnimations,
    bool? smartZoom,
    bool? aiNoiseRemoval,
    bool? aiAutoCaptions,
    bool? aiSilenceDetection,
  }) {
    return RecordingProfile(
      id: id,
      name: name,
      width: width ?? this.width,
      height: height ?? this.height,
      fps: fps ?? this.fps,
      encoder: encoder ?? this.encoder,
      micDeviceId: micDeviceId ?? this.micDeviceId,
      sysAudioDeviceId: sysAudioDeviceId ?? this.sysAudioDeviceId,
      webcamEnabled: webcamEnabled ?? this.webcamEnabled,
      webcamDeviceId: webcamDeviceId ?? this.webcamDeviceId,
      webcamX: webcamX ?? this.webcamX,
      webcamY: webcamY ?? this.webcamY,
      webcamWidth: webcamWidth ?? this.webcamWidth,
      webcamHeight: webcamHeight ?? this.webcamHeight,
      cursorHighlight: cursorHighlight ?? this.cursorHighlight,
      clickAnimations: clickAnimations ?? this.clickAnimations,
      smartZoom: smartZoom ?? this.smartZoom,
      zoomLevel: zoomLevel,
      aiNoiseRemoval: aiNoiseRemoval ?? this.aiNoiseRemoval,
      aiAutoCaptions: aiAutoCaptions ?? this.aiAutoCaptions,
      aiSilenceDetection: aiSilenceDetection ?? this.aiSilenceDetection,
    );
  }
}

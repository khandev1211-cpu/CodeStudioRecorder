import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';
import 'package:go_router/go_router.dart';

class SettingsScreen extends ConsumerStatefulWidget {
  const SettingsScreen({super.key});

  @override
  ConsumerState<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends ConsumerState<SettingsScreen> {
  late TextEditingController _fpsController;
  late String _outputPath;
  late bool _cursorHighlight;

  @override
  void initState() {
    super.initState();
    final service = ref.read(recordingServiceProvider);
    _fpsController = TextEditingController(
      text: service.getSettingInt("fps", 60).toString(),
    );
    _outputPath = service.getSettingString("output_path", "recordings");
    _cursorHighlight = service.getSettingInt("cursor_highlight", 1) == 1;
    
    // Sync initial state to engine AFTER the build is definitely done
    Future.microtask(() {
       service.setProcessorEnabled(0, _cursorHighlight);
    });
  }

  @override
  Widget build(BuildContext context) {
    final service = ref.read(recordingServiceProvider);

    return Scaffold(
      appBar: AppBar(
        title: const Text('Settings'),
        leading: IconButton(
          icon: const Icon(Icons.arrow_back),
          onPressed: () => context.go('/'),
        ),
      ),
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          const Text("Recording Settings", style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold)),
          const SizedBox(height: 20),
          ListTile(
            title: const Text("Target FPS"),
            subtitle: const Text("Preferred frames per second"),
            trailing: SizedBox(
              width: 60,
              child: TextField(
                controller: _fpsController,
                keyboardType: TextInputType.number,
                textAlign: TextAlign.center,
                onChanged: (val) {
                  final fps = int.tryParse(val);
                  if (fps != null) {
                    service.setSettingInt("fps", fps);
                  }
                },
              ),
            ),
          ),
          ListTile(
            title: const Text("Output Folder"),
            subtitle: Text(_outputPath),
            trailing: const Icon(Icons.folder_open),
            onTap: () {
              service.setSettingString("output_path", "C:/Users/Public/Videos");
              setState(() {
                _outputPath = "C:/Users/Public/Videos";
              });
            },
          ),
          const Divider(),
          const Text("Effects", style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold)),
          const SizedBox(height: 10),
          SwitchListTile(
            title: const Text("Cursor Highlight"),
            subtitle: const Text("Draw a yellow circle around the cursor"),
            value: _cursorHighlight,
            onChanged: (val) {
              setState(() {
                _cursorHighlight = val;
              });
              service.setSettingInt("cursor_highlight", val ? 1 : 0);
              service.setProcessorEnabled(0, val);
            },
          ),
        ],
      ),
    );
  }
}

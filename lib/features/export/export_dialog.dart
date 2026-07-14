import 'package:flutter/material.dart';
import 'package:codestudio_recorder/core/models/recording.dart';

class ExportDialog extends StatefulWidget {
  final Recording recording;
  const ExportDialog({super.key, required this.recording});

  @override
  State<ExportDialog> createState() => _ExportDialogState();
}

class _ExportDialogState extends State<ExportDialog> {
  String _selectedPreset = "Standard (MP4)";
  bool _removeSilence = true;
  bool _generateShorts = false;
  bool _smartFocus = true;

  final Map<String, String> _presets = {
    "Standard (MP4)": "Original resolution, high bitrate",
    "YouTube (1080p)": "Optimized for YouTube upload",
    "YouTube Shorts": "Vertical 9:16 crop, optimized",
    "Instagram Reels": "Vertical 9:16 crop, mobile optimized",
  };

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      backgroundColor: const Color(0xFF161616),
      title: const Row(
        children: [
          Icon(Icons.auto_fix_high, color: Colors.blueAccent),
          SizedBox(width: 12),
          Text("CREATOR EXPORT ENGINE"),
        ],
      ),
      content: SizedBox(
        width: 500,
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text("Recording: ${widget.recording.title}", style: const TextStyle(color: Colors.white54)),
            const Divider(height: 32, color: Colors.white10),
            
            const Text("AI ENHANCEMENTS", style: TextStyle(fontSize: 10, fontWeight: FontWeight.bold, letterSpacing: 2, color: Colors.blueAccent)),
            const SizedBox(height: 12),
            SwitchListTile(
              title: const Text("Smart Auto-Cut", style: TextStyle(fontSize: 14)),
              subtitle: const Text("Automatically remove silent gaps and dead air"),
              value: _removeSilence,
              onChanged: (val) => setState(() => _removeSilence = val),
              secondary: const Icon(Icons.content_cut, size: 20),
            ),
            SwitchListTile(
              title: const Text("Generate Dual Format", style: TextStyle(fontSize: 14)),
              subtitle: const Text("Create both 16:9 (Long) and 9:16 (Shorts) versions"),
              value: _generateShorts,
              onChanged: (val) => setState(() => _generateShorts = val),
              secondary: const Icon(Icons.layers, size: 20),
            ),
            
            const Divider(height: 32, color: Colors.white10),
            const Text("PRIMARY EXPORT PRESET", style: TextStyle(fontSize: 10, fontWeight: FontWeight.bold, letterSpacing: 2, color: Colors.white30)),
            const SizedBox(height: 12),
            Container(
              decoration: BoxDecoration(
                color: Colors.white.withOpacity(0.03),
                borderRadius: BorderRadius.circular(12),
              ),
              child: Column(
                children: _presets.keys.map((preset) => RadioListTile<String>(
                  title: Text(preset, style: const TextStyle(fontSize: 14)),
                  subtitle: Text(_presets[preset]!, style: const TextStyle(fontSize: 11)),
                  value: preset,
                  activeColor: Colors.blueAccent,
                  groupValue: _selectedPreset,
                  onChanged: (val) => setState(() => _selectedPreset = val!),
                )).toList(),
              ),
            ),
          ],
        ),
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(context),
          child: const Text("DISCARD", style: TextStyle(color: Colors.white38)),
        ),
        ElevatedButton.icon(
          onPressed: () {
            Navigator.pop(context);
            _processExport();
          },
          icon: const Icon(Icons.rocket_launch),
          label: const Text("RUN AUTO-CUT & EXPORT"),
          style: ElevatedButton.styleFrom(
            backgroundColor: Colors.blueAccent,
            foregroundColor: Colors.white,
            padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 16),
          ),
        ),
      ],
    );
  }

  void _processExport() {
    String message = "Processing ${_selectedPreset}...";
    if (_removeSilence) message += "\nRemoving silence...";
    if (_generateShorts) message += "\nCropping for YouTube Shorts...";
    
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(message),
        duration: const Duration(seconds: 4),
        behavior: SnackBarBehavior.floating,
        backgroundColor: Colors.blueGrey.shade900,
      ),
    );
  }
}

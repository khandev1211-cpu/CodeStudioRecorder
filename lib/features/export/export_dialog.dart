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

  final Map<String, String> _presets = {
    "Standard (MP4)": "Original resolution, high bitrate",
    "YouTube (1080p)": "Optimized for YouTube upload",
    "YouTube Shorts": "Vertical 9:16 crop, optimized",
    "Instagram Reels": "Vertical 9:16 crop, mobile optimized",
    "Animated GIF": "Low framerate, small file size",
  };

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: const Text("Recording Complete"),
      content: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text("File: ${widget.recording.title}"),
          const SizedBox(height: 20),
          const Text("Select Export Preset:", style: TextStyle(fontWeight: FontWeight.bold)),
          const SizedBox(height: 12),
          ..._presets.keys.map((preset) => RadioListTile<String>(
            title: Text(preset),
            subtitle: Text(_presets[preset]!),
            value: preset,
            groupValue: _selectedPreset,
            onChanged: (val) => setState(() => _selectedPreset = val!),
          )),
        ],
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(context),
          child: const Text("DISCARD"),
        ),
        ElevatedButton(
          onPressed: () {
            // Logic for processing export would go here
            Navigator.pop(context);
            ScaffoldMessenger.of(context).showSnackBar(
              SnackBar(content: Text("Exporting as $_selectedPreset...")),
            );
          },
          child: const Text("EXPORT & SAVE"),
        ),
      ],
    );
  }
}

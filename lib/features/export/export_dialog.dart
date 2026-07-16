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
  RangeValues _trimRange = const RangeValues(0, 1);
  double _totalDurationSeconds = 1;

  @override
  void initState() {
    super.initState();
    _totalDurationSeconds = widget.recording.duration.inSeconds.toDouble();
    if (_totalDurationSeconds < 1) _totalDurationSeconds = 1;
    _trimRange = RangeValues(0, _totalDurationSeconds);
  }

  final Map<String, String> _presets = {
    "Standard (MP4)": "Original resolution, high bitrate",
    "YouTube (1080p)": "Optimized for YouTube upload",
    "YouTube Shorts": "Vertical 9:16 crop, optimized",
  };

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      backgroundColor: const Color(0xFF161616),
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20), side: const BorderSide(color: Colors.white10)),
      title: const Row(
        children: [
          Icon(Icons.auto_fix_high, color: Colors.blueAccent),
          SizedBox(width: 12),
          Text("CREATOR EXPORT ENGINE"),
        ],
      ),
      content: SizedBox(
        width: 550,
        child: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text("Recording: ${widget.recording.title}", style: const TextStyle(color: Colors.white54, fontSize: 13)),
              const Divider(height: 32, color: Colors.white10),
              
              const Text("1. CLIP TRIMMER", style: TextStyle(fontSize: 10, fontWeight: FontWeight.bold, letterSpacing: 2, color: Colors.orangeAccent)),
              const SizedBox(height: 16),
              Column(
                children: [
                  RangeSlider(
                    values: _trimRange,
                    min: 0,
                    max: _totalDurationSeconds,
                    activeColor: Colors.orangeAccent,
                    inactiveColor: Colors.white10,
                    onChanged: (val) => setState(() => _trimRange = val),
                  ),
                  Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Text(_formatTime(_trimRange.start), style: const TextStyle(fontFamily: 'monospace', color: Colors.orangeAccent)),
                      const Text("Selected Segment", style: TextStyle(fontSize: 11, color: Colors.white24)),
                      Text(_formatTime(_trimRange.end), style: const TextStyle(fontFamily: 'monospace', color: Colors.orangeAccent)),
                    ],
                  ),
                ],
              ),

              const Divider(height: 40, color: Colors.white10),
              const Text("2. AI ENHANCEMENTS", style: TextStyle(fontSize: 10, fontWeight: FontWeight.bold, letterSpacing: 2, color: Colors.blueAccent)),
              const SizedBox(height: 12),
              SwitchListTile(
                title: const Text("Smart Auto-Cut", style: TextStyle(fontSize: 14)),
                subtitle: const Text("Automatically remove silent gaps"),
                value: _removeSilence,
                onChanged: (val) => setState(() => _removeSilence = val),
                secondary: const Icon(Icons.content_cut, size: 20),
              ),
              SwitchListTile(
                title: const Text("Generate Shorts (9:16)", style: TextStyle(fontSize: 14)),
                subtitle: const Text("Auto-crop for vertical platforms"),
                value: _generateShorts,
                onChanged: (val) => setState(() => _generateShorts = val),
                secondary: const Icon(Icons.mobile_screen_share, size: 20),
              ),
              
              const Divider(height: 32, color: Colors.white10),
              const Text("3. EXPORT PRESET", style: TextStyle(fontSize: 10, fontWeight: FontWeight.bold, letterSpacing: 2, color: Colors.white30)),
              const SizedBox(height: 12),
              ..._presets.keys.map((preset) => RadioListTile<String>(
                title: Text(preset, style: const TextStyle(fontSize: 14)),
                value: preset,
                activeColor: Colors.blueAccent,
                groupValue: _selectedPreset,
                onChanged: (val) => setState(() => _selectedPreset = val!),
              )),
            ],
          ),
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
          label: const Text("EXPORT & FINALIZE"),
          style: ElevatedButton.styleFrom(
            backgroundColor: Colors.blueAccent,
            foregroundColor: Colors.white,
            padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 16),
          ),
        ),
      ],
    );
  }

  String _formatTime(double seconds) {
    final dur = Duration(seconds: seconds.toInt());
    return dur.toString().split('.').first.padLeft(8, "0");
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

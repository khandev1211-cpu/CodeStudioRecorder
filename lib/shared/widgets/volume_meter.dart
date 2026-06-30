import 'package:flutter/material.dart';

class VolumeMeter extends StatelessWidget {
  final double level; // 0.0 to 1.0
  final String label;
  final Color color;

  const VolumeMeter({
    super.key,
    required this.level,
    required this.label,
    this.color = Colors.greenAccent,
  });

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(label, style: const TextStyle(fontSize: 10, color: Colors.white54)),
            Text('${(level * 100).toInt()}%', style: const TextStyle(fontSize: 10, color: Colors.white54)),
          ],
        ),
        const SizedBox(height: 4),
        ClipRRect(
          borderRadius: BorderRadius.circular(2),
          child: LinearProgressIndicator(
            value: level,
            minHeight: 6,
            backgroundColor: Colors.white10,
            color: level > 0.8 ? Colors.redAccent : (level > 0.6 ? Colors.orangeAccent : color),
          ),
        ),
      ],
    );
  }
}

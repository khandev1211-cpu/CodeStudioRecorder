import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';
import 'package:codestudio_recorder/features/recording/annotation_state.dart';

class AnnotationToolbar extends ConsumerWidget {
  const AnnotationToolbar({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final state = ref.watch(annotationProvider);
    final notifier = ref.read(annotationProvider.notifier);
    final service = ref.read(recordingServiceProvider);

    return Card(
      elevation: 8,
      color: Colors.black87,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(30)),
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
        child: Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            _ToolButton(
              icon: Icons.horizontal_rule,
              isSelected: state.tool == AnnotationTool.line,
              onPressed: () => notifier.setTool(AnnotationTool.line),
              tooltip: 'Line',
            ),
            _ToolButton(
              icon: Icons.check_box_outline_blank,
              isSelected: state.tool == AnnotationTool.rect,
              onPressed: () => notifier.setTool(AnnotationTool.rect),
              tooltip: 'Rectangle',
            ),
            _ToolButton(
              icon: Icons.panorama_fish_eye,
              isSelected: state.tool == AnnotationTool.ellipse,
              onPressed: () => notifier.setTool(AnnotationTool.ellipse),
              tooltip: 'Ellipse',
            ),
            _ToolButton(
              icon: Icons.arrow_outward,
              isSelected: state.tool == AnnotationTool.arrow,
              onPressed: () => notifier.setTool(AnnotationTool.arrow),
              tooltip: 'Arrow',
            ),
            const VerticalDivider(color: Colors.white24, width: 20),
            _ColorButton(
              color: Colors.red,
              isSelected: state.color == Colors.red,
              onPressed: () => notifier.setColor(Colors.red),
            ),
            _ColorButton(
              color: Colors.yellow,
              isSelected: state.color == Colors.yellow,
              onPressed: () => notifier.setColor(Colors.yellow),
            ),
            _ColorButton(
              color: Colors.green,
              isSelected: state.color == Colors.green,
              onPressed: () => notifier.setColor(Colors.green),
            ),
            const VerticalDivider(color: Colors.white24, width: 20),
            IconButton(
              icon: const Icon(Icons.undo, color: Colors.white),
              onPressed: () => service.undoAnnotation(),
              tooltip: 'Undo',
            ),
            IconButton(
              icon: const Icon(Icons.delete_sweep, color: Colors.white),
              onPressed: () => service.clearAnnotations(),
              tooltip: 'Clear All',
            ),
            IconButton(
              icon: const Icon(Icons.close, color: Colors.white54),
              onPressed: () => notifier.setTool(AnnotationTool.none),
              tooltip: 'Close Tool',
            ),
          ],
        ),
      ),
    );
  }
}

class _ToolButton extends StatelessWidget {
  final IconData icon;
  final bool isSelected;
  final VoidCallback onPressed;
  final String tooltip;

  const _ToolButton({
    required this.icon,
    required this.isSelected,
    required this.onPressed,
    required this.tooltip,
  });

  @override
  Widget build(BuildContext context) {
    return IconButton(
      icon: Icon(icon, color: isSelected ? const Color(0xFFFF3B3B) : Colors.white70),
      onPressed: onPressed,
      tooltip: tooltip,
    );
  }
}

class _ColorButton extends StatelessWidget {
  final Color color;
  final bool isSelected;
  final VoidCallback onPressed;

  const _ColorButton({
    required this.color,
    required this.isSelected,
    required this.onPressed,
  });

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onPressed,
      child: Container(
        margin: const EdgeInsets.symmetric(horizontal: 4),
        width: 20,
        height: 20,
        decoration: BoxDecoration(
          color: color,
          shape: BoxShape.circle,
          border: isSelected ? Border.all(color: Colors.white, width: 2) : null,
        ),
      ),
    );
  }
}

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

enum AnnotationTool { line, rect, ellipse, arrow, none }

class AnnotationState {
  final AnnotationTool tool;
  final Color color;
  final double strokeWidth;

  AnnotationState({
    this.tool = AnnotationTool.none,
    this.color = Colors.red,
    this.strokeWidth = 4.0,
  });

  AnnotationState copyWith({
    AnnotationTool? tool,
    Color? color,
    double? strokeWidth,
  }) {
    return AnnotationState(
      tool: tool ?? this.tool,
      color: color ?? this.color,
      strokeWidth: strokeWidth ?? this.strokeWidth,
    );
  }
}

class AnnotationNotifier extends StateNotifier<AnnotationState> {
  AnnotationNotifier() : super(AnnotationState());

  void setTool(AnnotationTool tool) => state = state.copyWith(tool: tool);
  void setColor(Color color) => state = state.copyWith(color: color);
  void setStrokeWidth(double width) => state = state.copyWith(strokeWidth: width);
}

final annotationProvider = StateNotifierProvider<AnnotationNotifier, AnnotationState>((ref) {
  return AnnotationNotifier();
});

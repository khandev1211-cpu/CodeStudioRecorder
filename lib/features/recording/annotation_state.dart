import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';

enum AnnotationTool { line, rect, ellipse, arrow, none }

class AnnotationState {
  final AnnotationTool tool;
  final Color color;
  final double strokeWidth;
  final bool zoomEnabled;
  final double zoomLevel;
  final bool webcamEnabled;

  AnnotationState({
    this.tool = AnnotationTool.none,
    this.color = Colors.red,
    this.strokeWidth = 4.0,
    this.zoomEnabled = false,
    this.zoomLevel = 1.5,
    this.webcamEnabled = false,
  });

  AnnotationState copyWith({
    AnnotationTool? tool,
    Color? color,
    double? strokeWidth,
    bool? zoomEnabled,
    double? zoomLevel,
    bool? webcamEnabled,
  }) {
    return AnnotationState(
      tool: tool ?? this.tool,
      color: color ?? this.color,
      strokeWidth: strokeWidth ?? this.strokeWidth,
      zoomEnabled: zoomEnabled ?? this.zoomEnabled,
      zoomLevel: zoomLevel ?? this.zoomLevel,
      webcamEnabled: webcamEnabled ?? this.webcamEnabled,
    );
  }
}

class AnnotationNotifier extends StateNotifier<AnnotationState> {
  final RecordingService _service;
  AnnotationNotifier(this._service) : super(AnnotationState());

  void setTool(AnnotationTool tool) {
    state = state.copyWith(tool: tool);
    // Automatically enable the native annotation processor when a tool is selected
    _service.setProcessorEnabled(2, tool != AnnotationTool.none);
  }
  void setColor(Color color) => state = state.copyWith(color: color);
  void setStrokeWidth(double width) => state = state.copyWith(strokeWidth: width);
  
  void toggleZoom() {
    final newState = !state.zoomEnabled;
    state = state.copyWith(zoomEnabled: newState);
    _service.setProcessorEnabled(3, newState);
  }

  void setZoomLevel(double level) {
    state = state.copyWith(zoomLevel: level);
    _service.setZoomLevel(level);
  }

  void toggleWebcam() {
    final newState = !state.webcamEnabled;
    state = state.copyWith(webcamEnabled: newState);
    _service.setProcessorEnabled(4, newState);
  }
}

final annotationProvider = StateNotifierProvider<AnnotationNotifier, AnnotationState>((ref) {
  final service = ref.read(recordingServiceProvider);
  return AnnotationNotifier(service);
});

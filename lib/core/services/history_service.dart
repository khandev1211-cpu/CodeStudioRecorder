import 'dart:convert';
import 'dart:io';
import 'package:codestudio_recorder/core/models/recording.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:path/path.dart' as p;

final historyServiceProvider = Provider((ref) => HistoryService());

class HistoryService {
  List<Recording> _history = [];

  Future<void> loadHistory() async {
    try {
      final file = await _getHistoryFile();
      if (await file.exists()) {
        final content = await file.readAsString();
        final List<dynamic> jsonList = json.decode(content);
        _history = jsonList.map((e) => Recording.fromJson(e)).toList();
      }
    } catch (e) {
      // In a production app, we would log this to a file
    }
  }

  Future<void> saveRecording(Recording recording) async {
    _history.insert(0, recording);
    await _persist();
  }

  Future<void> deleteRecording(String id) async {
    _history.removeWhere((r) => r.id == id);
    await _persist();
  }

  List<Recording> get history => List.unmodifiable(_history);

  Future<void> _persist() async {
    final file = await _getHistoryFile();
    final jsonList = _history.map((e) => e.toJson()).toList();
    if (!await file.parent.exists()) {
      await file.parent.create(recursive: true);
    }
    await file.writeAsString(json.encode(jsonList));
  }

  Future<File> _getHistoryFile() async {
    // Manually find the AppData folder to avoid broken path_provider dependencies
    final appData = Platform.environment['APPDATA'];
    if (appData == null) {
      // Fallback to local directory if APPDATA is missing (unlikely on Windows)
      return File('codestudio_history.json');
    }
    return File(p.join(appData, 'CodeStudioRecorder', 'data', 'history.json'));
  }
}

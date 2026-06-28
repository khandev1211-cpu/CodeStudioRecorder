import 'dart:convert';
import 'dart:io';
import 'package:path_provider/path_provider.dart';
import 'package:codestudio_recorder/core/models/recording.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

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
      print("Error loading history: $e");
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
    await file.writeAsString(json.encode(jsonList));
  }

  Future<File> _getHistoryFile() async {
    final dir = await getApplicationDocumentsDirectory();
    return File('${dir.path}/codestudio_history.json');
  }
}

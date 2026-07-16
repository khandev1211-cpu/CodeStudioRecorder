import 'dart:io';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/history_service.dart';
import 'package:go_router/go_router.dart';
import 'package:intl/intl.dart';
import 'package:url_launcher/url_launcher.dart';

class HistoryScreen extends ConsumerWidget {
  const HistoryScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final history = ref.watch(historyServiceProvider).history;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Recording History'),
        leading: IconButton(
          icon: const Icon(Icons.arrow_back),
          onPressed: () => context.go('/'),
        ),
      ),
      body: history.isEmpty
          ? const Center(
              child: Text('No recordings yet.'),
            )
          : ListView.builder(
              padding: const EdgeInsets.all(16),
              itemCount: history.length,
              itemBuilder: (context, index) {
                final recording = history[index];
                return Card(
                  margin: const EdgeInsets.only(bottom: 12),
                  child: ListTile(
                    leading: Container(
                      width: 64,
                      height: 36,
                      decoration: BoxDecoration(
                        color: Colors.black,
                        borderRadius: BorderRadius.circular(4),
                      ),
                      child: recording.thumbnailPath != null && File(recording.thumbnailPath!).existsSync()
                          ? ClipRRect(
                              borderRadius: BorderRadius.circular(4),
                              child: Image.file(File(recording.thumbnailPath!), fit: BoxFit.cover),
                            )
                          : const Icon(Icons.movie, color: Colors.red),
                    ),
                    title: Text(recording.title),
                    subtitle: Text(
                      '${DateFormat.yMMMd().add_jm().format(recording.createdAt)} • ${_formatDuration(recording.duration)}',
                    ),
                    trailing: Row(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        IconButton(
                          icon: const Icon(Icons.folder_open),
                          tooltip: 'Open Folder',
                          onPressed: () => _openFolder(recording.filePath),
                        ),
                        IconButton(
                          icon: const Icon(Icons.play_arrow),
                          tooltip: 'Play',
                          onPressed: () => _playFile(recording.filePath),
                        ),
                      ],
                    ),
                  ),
                );
              },
            ),
    );
  }

  Future<void> _openFolder(String filePath) async {
    final file = File(filePath);
    final folder = file.parent;
    if (await folder.exists()) {
      // On Windows, explorer /select,path is best but launchUrl works for directory
      final uri = Uri.directory(folder.path);
      await launchUrl(uri);
    }
  }

  Future<void> _playFile(String filePath) async {
    final file = File(filePath);
    if (await file.exists()) {
      final uri = Uri.file(file.path);
      if (!await launchUrl(uri)) {
        // If it's a dummy text file, it might not "play" as video, 
        // but launchUrl should open it in Notepad
      }
    }
  }

  String _formatDuration(Duration duration) {
    String twoDigits(int n) => n.toString().padLeft(2, "0");
    String twoDigitMinutes = twoDigits(duration.inMinutes.remainder(60));
    String twoDigitSeconds = twoDigits(duration.inSeconds.remainder(60));
    if (duration.inHours > 0) {
      return "${twoDigits(duration.inHours)}:$twoDigitMinutes:$twoDigitSeconds";
    }
    return "$twoDigitMinutes:$twoDigitSeconds";
  }
}

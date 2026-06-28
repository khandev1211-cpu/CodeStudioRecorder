import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/app.dart';
import 'package:codestudio_recorder/core/services/history_service.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  final container = ProviderContainer();
  await container.read(historyServiceProvider).loadHistory();

  runApp(
    UncontrolledProviderScope(
      container: container,
      child: const CodeStudioApp(),
    ),
  );
}

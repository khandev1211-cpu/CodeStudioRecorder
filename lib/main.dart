import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/features/home/home_screen.dart';

void main() {
  runApp(
    const ProviderScope(
      child: CodeStudioApp(),
    ),
  );
}

class CodeStudioApp extends StatelessWidget {
  const CodeStudioApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'CodeStudio Recorder',
      theme: ThemeData(
        brightness: Brightness.dark,
        primarySwatch: Colors.red,
        useMaterial3: true,
      ),
      home: const HomeScreen(),
    );
  }
}

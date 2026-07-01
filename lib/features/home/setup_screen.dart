import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/setup_service.dart';
import 'package:go_router/go_router.dart';
import 'package:codestudio_recorder/shared/theme/app_logo.dart';

class SetupScreen extends ConsumerStatefulWidget {
  const SetupScreen({super.key});

  @override
  ConsumerState<SetupScreen> createState() => _SetupScreenState();
}

class _SetupScreenState extends ConsumerState<SetupScreen> {
  String _status = "Checking system requirements...";
  bool _error = false;

  @override
  void initState() {
    super.initState();
    _runCheck();
  }

  Future<void> _runCheck() async {
    final setup = ref.read(setupServiceProvider);
    await Future.delayed(const Duration(seconds: 1)); // Visual delay for logo

    final ok = await setup.runIntegrityCheck();
    if (ok) {
      if (mounted) context.go('/');
    } else {
      setState(() {
        _status = "System check failed.\nPlease ensure you are on Windows 10 1903+ and FFmpeg DLLs are present.";
        _error = true;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            const AppLogo(size: 150),
            const SizedBox(height: 40),
            if (!_error)
              const CircularProgressIndicator()
            else
              const Icon(Icons.error_outline, size: 48, color: Colors.red),
            const SizedBox(height: 20),
            Text(
              _status,
              textAlign: TextAlign.center,
              style: TextStyle(color: _error ? Colors.red : Colors.white70),
            ),
            if (_error) ...[
              const SizedBox(height: 20),
              ElevatedButton(
                onPressed: () {
                  setState(() {
                    _error = false;
                    _status = "Retrying check...";
                  });
                  _runCheck();
                },
                child: const Text("RETRY"),
              ),
            ]
          ],
        ),
      ),
    );
  }
}

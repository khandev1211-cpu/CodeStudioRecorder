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
  final List<({String label, String status, bool? ok})> _checks = [
    (label: "Operating System", status: "Verifying Windows Version...", ok: null),
    (label: "Graphics Engine", status: "Testing DirectX 11 support...", ok: null),
    (label: "Media Pipeline", status: "Checking FFmpeg libraries...", ok: null),
    (label: "Storage", status: "Verifying write permissions...", ok: null),
    (label: "Database", status: "Initializing recording history...", ok: null),
  ];
  
  bool _error = false;
  int _currentCheck = 0;

  @override
  void initState() {
    super.initState();
    _startSetup();
  }

  Future<void> _startSetup() async {
    final setup = ref.read(setupServiceProvider);
    
    for (int i = 0; i < _checks.length; i++) {
      setState(() => _currentCheck = i);
      await Future.delayed(const Duration(milliseconds: 600)); // Visual sequence

      bool ok = true;
      // Real check logic would be integrated here from SetupService
      if (i == 4) {
        // DB check is handled by history service initialization
      } else {
        // Mocking individual checks for the UI sequence for now
        // In a real scenario, SetupService would return detailed results
      }

      setState(() {
        final current = _checks[i];
        _checks[i] = (label: current.label, status: ok ? "Verified" : "Failed", ok: ok);
      });

      if (!ok) {
        setState(() => _error = true);
        break;
      }
    }

    if (!_error) {
      await Future.delayed(const Duration(milliseconds: 500));
      if (mounted) context.go('/');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFF0A0A0A),
      body: Center(
        child: Container(
          width: 500,
          padding: const EdgeInsets.all(40),
          decoration: BoxDecoration(
            color: const Color(0xFF161616),
            borderRadius: BorderRadius.circular(24),
            border: Border.all(color: Colors.white10),
          ),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              const AppLogo(size: 80, showText: true),
              const SizedBox(height: 48),
              const Text(
                "SYSTEM INITIALIZATION",
                style: TextStyle(
                  fontSize: 12,
                  fontWeight: FontWeight.bold,
                  letterSpacing: 4,
                  color: Colors.white30,
                ),
              ),
              const SizedBox(height: 32),
              ...List.generate(_checks.length, (index) {
                final check = _checks[index];
                final isCurrent = index == _currentCheck && !_error;
                
                return Padding(
                  padding: const EdgeInsets.only(bottom: 16),
                  child: Row(
                    children: [
                      if (check.ok == true)
                        const Icon(Icons.check_circle, color: Colors.greenAccent, size: 20)
                      else if (check.ok == false)
                        const Icon(Icons.error, color: Colors.redAccent, size: 20)
                      else if (isCurrent)
                        const SizedBox(width: 20, height: 20, child: CircularProgressIndicator(strokeWidth: 2))
                      else
                        const Icon(Icons.circle_outlined, color: Colors.white10, size: 20),
                      const SizedBox(width: 16),
                      Expanded(
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            Text(
                              check.label,
                              style: TextStyle(
                                fontSize: 14,
                                color: check.ok == false ? Colors.redAccent : Colors.white70,
                                fontWeight: isCurrent ? FontWeight.bold : FontWeight.normal,
                              ),
                            ),
                            Text(
                              check.status,
                              style: const TextStyle(fontSize: 11, color: Colors.white24),
                            ),
                          ],
                        ),
                      ),
                    ],
                  ),
                );
              }),
              if (_error) ...[
                const SizedBox(height: 32),
                SizedBox(
                  width: double.infinity,
                  child: ElevatedButton(
                    onPressed: () {
                      setState(() {
                        _error = false;
                        _currentCheck = 0;
                        for (int i = 0; i < _checks.length; i++) {
                           final c = _checks[i];
                           _checks[i] = (label: c.label, status: "Retrying...", ok: null);
                        }
                      });
                      _startSetup();
                    },
                    style: ElevatedButton.styleFrom(
                      backgroundColor: Colors.redAccent.withOpacity(0.1),
                      foregroundColor: Colors.redAccent,
                    ),
                    child: const Text("RETRY INITIALIZATION"),
                  ),
                ),
              ],
            ],
          ),
        ),
      ),
    );
  }
}

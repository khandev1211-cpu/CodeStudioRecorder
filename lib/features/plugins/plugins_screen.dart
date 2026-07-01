import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/recording_service.dart';
import 'package:go_router/go_router.dart';

class PluginsScreen extends ConsumerStatefulWidget {
  const PluginsScreen({super.key});

  @override
  ConsumerState<PluginsScreen> createState() => _PluginsScreenState();
}

class _PluginsScreenState extends ConsumerState<PluginsScreen> {
  int _pluginCount = 0;
  final List<bool> _pluginStatus = [];

  @override
  void initState() {
    super.initState();
    _refreshPlugins();
  }

  void _refreshPlugins() {
    final service = ref.read(recordingServiceProvider);
    final count = service.getPluginCount();
    setState(() {
      _pluginCount = count;
      _pluginStatus.clear();
      for (int i = 0; i < count; i++) {
        _pluginStatus.add(false); // Default to off
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    final service = ref.read(recordingServiceProvider);

    return Scaffold(
      appBar: AppBar(
        title: const Text('Plugins'),
        leading: IconButton(
          icon: const Icon(Icons.arrow_back),
          onPressed: () => context.go('/'),
        ),
        actions: [
          IconButton(
            icon: const Icon(Icons.refresh),
            onPressed: _refreshPlugins,
          ),
        ],
      ),
      body: _pluginCount == 0
          ? const Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Icon(Icons.extension_off, size: 64, color: Colors.white24),
                  SizedBox(height: 16),
                  Text('No plugins found in /plugins directory.', style: TextStyle(color: Colors.white54)),
                ],
              ),
            )
          : ListView.builder(
              padding: const EdgeInsets.all(16),
              itemCount: _pluginCount,
              itemBuilder: (context, index) {
                return Card(
                  child: ListTile(
                    leading: const Icon(Icons.extension, color: Colors.blueAccent),
                    title: Text('Plugin #$index'),
                    subtitle: const Text('External Frame Processor'),
                    trailing: Switch(
                      value: _pluginStatus[index],
                      onChanged: (val) {
                        setState(() {
                          _pluginStatus[index] = val;
                        });
                        service.setPluginEnabled(index, val);
                      },
                    ),
                  ),
                );
              },
            ),
      floatingActionButton: FloatingActionButton.extended(
        onPressed: () => _showPluginInfo(context),
        label: const Text('Install Plugin'),
        icon: const Icon(Icons.add),
      ),
    );
  }

  void _showPluginInfo(BuildContext context) {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('How to install plugins'),
        content: const Text(
          'Place your plugin .dll files in the "plugins" directory relative to the application executable. '
          'CodeStudio will automatically detect and load them at startup.',
        ),
        actions: [
          TextButton(onPressed: () => Navigator.pop(context), child: const Text('OK')),
        ],
      ),
    );
  }
}

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
  final List<({String name, String description, String author, String version})> _pluginInfos = [];

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
      _pluginInfos.clear();
      for (int i = 0; i < count; i++) {
        _pluginStatus.add(false); // Default to off
        _pluginInfos.add(service.getPluginInfo(i));
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    final service = ref.read(recordingServiceProvider);

    return Scaffold(
      appBar: AppBar(
        title: const Text('Plugin Marketplace'),
        backgroundColor: Colors.transparent,
        elevation: 0,
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
          ? Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Icon(Icons.extension_off_outlined, size: 64, color: Colors.white10),
                  const SizedBox(height: 16),
                  const Text('No plugins detected.', style: TextStyle(color: Colors.white24, fontSize: 16)),
                  const SizedBox(height: 8),
                  const Text('Place .dll files in the /plugins folder.', style: TextStyle(color: Colors.white10, fontSize: 12)),
                ],
              ),
            )
          : ListView.builder(
              padding: const EdgeInsets.all(24),
              itemCount: _pluginCount,
              itemBuilder: (context, index) {
                final info = _pluginInfos[index];
                return Card(
                  margin: const EdgeInsets.only(bottom: 16),
                  shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12), side: const BorderSide(color: Colors.white10)),
                  child: Padding(
                    padding: const EdgeInsets.all(8.0),
                    child: ListTile(
                      leading: Container(
                        padding: const EdgeInsets.all(12),
                        decoration: BoxDecoration(color: Colors.blueAccent.withOpacity(0.1), borderRadius: BorderRadius.circular(8)),
                        child: const Icon(Icons.extension, color: Colors.blueAccent),
                      ),
                      title: Text(info.name, style: const TextStyle(fontWeight: FontWeight.bold)),
                      subtitle: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          const SizedBox(height: 4),
                          Text(info.description, style: const TextStyle(color: Colors.white54, fontSize: 13)),
                          const SizedBox(height: 8),
                          Row(
                            children: [
                              Text("v${info.version}", style: const TextStyle(color: Colors.white24, fontSize: 11)),
                              const SizedBox(width: 12),
                              Text("by ${info.author}", style: const TextStyle(color: Colors.white24, fontSize: 11)),
                            ],
                          ),
                        ],
                      ),
                      trailing: Switch(
                        value: _pluginStatus[index],
                        activeColor: Colors.blueAccent,
                        onChanged: (val) {
                          setState(() => _pluginStatus[index] = val);
                          service.setPluginEnabled(index, val);
                        },
                      ),
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

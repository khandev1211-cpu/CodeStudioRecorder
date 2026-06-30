import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:codestudio_recorder/core/services/profile_service.dart';
import 'package:codestudio_recorder/core/models/recording_profile.dart';
import 'package:go_router/go_router.dart';

class ProfilesScreen extends ConsumerWidget {
  const ProfilesScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final profileService = ref.watch(profileServiceProvider);
    final profiles = profileService.profiles;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Recording Profiles'),
        leading: IconButton(
          icon: const Icon(Icons.arrow_back),
          onPressed: () => context.go('/'),
        ),
      ),
      body: ListView.builder(
        padding: const EdgeInsets.all(16),
        itemCount: profiles.length,
        itemBuilder: (context, index) {
          final profile = profiles[index];
          return Card(
            margin: const EdgeInsets.only(bottom: 12),
            child: ListTile(
              onTap: () => context.go('/profiles/${profile.id}'),
              leading: const Icon(Icons.description, color: Color(0xFFFF3B3B)),
              title: Text(profile.name),
              subtitle: Text(
                '${profile.width}x${profile.height} @ ${profile.fps}fps • '
                '${profile.smartZoom ? "Zoom ON" : "Zoom OFF"}',
              ),
              trailing: profile.id == 'default' 
                ? null 
                : IconButton(
                    icon: const Icon(Icons.delete),
                    onPressed: () => profileService.deleteProfile(profile.id),
                  ),
            ),
          );
        },
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: () => _showAddProfileDialog(context, ref),
        backgroundColor: const Color(0xFFFF3B3B),
        child: const Icon(Icons.add),
      ),
    );
  }

  void _showAddProfileDialog(BuildContext context, WidgetRef ref) {
    final nameController = TextEditingController();
    final profileService = ref.read(profileServiceProvider);

    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('New Profile'),
        content: TextField(
          controller: nameController,
          decoration: const InputDecoration(hintText: 'Profile Name'),
          autofocus: true,
        ),
        actions: [
          TextButton(onPressed: () => Navigator.pop(context), child: const Text('CANCEL')),
          ElevatedButton(
            onPressed: () async {
              if (nameController.text.isNotEmpty) {
                final newProfile = RecordingProfile(
                  id: DateTime.now().millisecondsSinceEpoch.toString(),
                  name: nameController.text,
                );
                await profileService.saveProfile(newProfile);
                Navigator.pop(context);
              }
            },
            child: const Text('CREATE'),
          ),
        ],
      ),
    );
  }
}

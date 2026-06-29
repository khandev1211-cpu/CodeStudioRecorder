import 'dart:convert';
import 'dart:io';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:path/path.dart' as p;
import 'package:codestudio_recorder/core/models/recording_profile.dart';

final profileServiceProvider = Provider((ref) => ProfileService());

final selectedProfileProvider = StateProvider<RecordingProfile>((ref) {
  final service = ref.watch(profileServiceProvider);
  return service.profiles.isNotEmpty ? service.profiles.first : RecordingProfile.defaultProfile();
});

class ProfileService {
  List<RecordingProfile> _profiles = [];

  Future<void> loadProfiles() async {
    try {
      final file = await _getProfilesFile();
      if (await file.exists()) {
        final content = await file.readAsString();
        final List<dynamic> jsonList = json.decode(content);
        _profiles = jsonList.map((e) => RecordingProfile.fromJson(e)).toList();
      }
      
      if (_profiles.isEmpty) {
        _profiles.add(RecordingProfile.defaultProfile());
        await _persist();
      }
    } catch (e) {
      print("Error loading profiles: $e");
    }
  }

  List<RecordingProfile> get profiles => List.unmodifiable(_profiles);

  Future<void> saveProfile(RecordingProfile profile) async {
    final index = _profiles.indexWhere((p) => p.id == profile.id);
    if (index >= 0) {
      _profiles[index] = profile;
    } else {
      _profiles.add(profile);
    }
    await _persist();
  }

  Future<void> deleteProfile(String id) async {
    if (id == 'default') return;
    _profiles.removeWhere((p) => p.id == id);
    await _persist();
  }

  Future<void> _persist() async {
    final file = await _getProfilesFile();
    if (!await file.parent.exists()) {
      await file.parent.create(recursive: true);
    }
    await file.writeAsString(json.encode(_profiles.map((e) => e.toJson()).toList()));
  }

  Future<File> _getProfilesFile() async {
    final appData = Platform.environment['APPDATA'];
    if (appData == null) return File('profiles.json');
    return File(p.join(appData, 'CodeStudioRecorder', 'data', 'profiles.json'));
  }
}

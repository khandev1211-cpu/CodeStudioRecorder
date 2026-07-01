import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:codestudio_recorder/features/home/home_screen.dart';
import 'package:codestudio_recorder/features/history/history_screen.dart';
import 'package:codestudio_recorder/features/settings/settings_screen.dart';
import 'package:codestudio_recorder/features/profiles/profiles_screen.dart';
import 'package:codestudio_recorder/features/profiles/profile_details_screen.dart';
import 'package:codestudio_recorder/features/plugins/plugins_screen.dart';
import 'package:codestudio_recorder/features/home/setup_screen.dart';

final _router = GoRouter(
  initialLocation: '/setup',
  routes: [
    GoRoute(
      path: '/setup',
      builder: (context, state) => const SetupScreen(),
    ),
    GoRoute(
      path: '/',
      builder: (context, state) => const HomeScreen(),
    ),
    GoRoute(
      path: '/history',
      builder: (context, state) => const HistoryScreen(),
    ),
    GoRoute(
      path: '/settings',
      builder: (context, state) => const SettingsScreen(),
    ),
    GoRoute(
      path: '/plugins',
      builder: (context, state) => const PluginsScreen(),
    ),
    GoRoute(
      path: '/profiles',
      builder: (context, state) => const ProfilesScreen(),
      routes: [
        GoRoute(
          path: ':id',
          builder: (context, state) => ProfileDetailsScreen(profileId: state.pathParameters['id']!),
        ),
      ],
    ),
  ],
);

class CodeStudioApp extends StatelessWidget {
  const CodeStudioApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp.router(
      title: 'CodeStudio Recorder',
      debugShowCheckedModeBanner: false,
      routerConfig: _router,
      theme: ThemeData(
        brightness: Brightness.dark,
        primaryColor: const Color(0xFFFF3B3B),
        scaffoldBackgroundColor: const Color(0xFF111111),
        colorScheme: const ColorScheme.dark(
          primary: Color(0xFFFF3B3B),
          surface: Color(0xFF1A1A1A),
        ),
        useMaterial3: true,
      ),
    );
  }
}

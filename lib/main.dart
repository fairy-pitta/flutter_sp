import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'screens/main_screen.dart';
import 'screens/settings_screen.dart';
import 'screens/export_screen.dart';
import 'services/audio_service.dart';
import 'services/spectrogram_service.dart';

void main() {
  runApp(MelSpectrogramApp());
}

class MelSpectrogramApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MultiProvider(
      providers: [
        ChangeNotifierProvider(create: (_) => AudioService()),
        ChangeNotifierProvider(create: (_) => SpectrogramService()),
      ],
      child: MaterialApp(
        title: 'Mel Spectrogram',
        theme: ThemeData(
          primaryColor: Color(0xFF1a1a1a),
          scaffoldBackgroundColor: Color(0xFF000000),
          colorScheme: ColorScheme.dark(
            primary: Color(0xFF00ff88),
            secondary: Color(0xFFff6600),
          ),
          textTheme: TextTheme(
            bodyLarge: TextStyle(color: Colors.white, fontSize: 14),
            bodyMedium: TextStyle(color: Colors.white, fontSize: 12),
          ),
        ),
        initialRoute: '/',
        routes: {
          '/': (context) => MainScreen(),
          '/settings': (context) => SettingsScreen(),
          '/export': (context) => ExportScreen(),
        },
      ),
    );
  }
}
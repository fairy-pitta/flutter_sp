import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:provider/provider.dart';
import 'package:mel_spectrogram/main.dart';
import 'package:mel_spectrogram/screens/main_screen.dart';
import 'package:mel_spectrogram/services/audio_service.dart';
import 'package:mel_spectrogram/services/spectrogram_service.dart';
import 'package:mel_spectrogram/widgets/spectrogram_widget.dart';
import 'package:mel_spectrogram/widgets/control_panel.dart';
import 'package:mel_spectrogram/widgets/status_bar.dart';

void main() {
  group('Mel Spectrogram App Widget Tests', () {
    testWidgets('App should launch with main screen', (WidgetTester tester) async {
      await tester.pumpWidget(MelSpectrogramApp());
      
      // Verify app title
      expect(find.text('Mel Spectrogram'), findsOneWidget);
      
      // Verify main screen is present
      expect(find.byType(MainScreen), findsOneWidget);
    });
    
    testWidgets('Main screen should have all required components', (WidgetTester tester) async {
      await tester.pumpWidget(
        MultiProvider(
          providers: [
            ChangeNotifierProvider(create: (_) => AudioService()),
            ChangeNotifierProvider(create: (_) => SpectrogramService()),
          ],
          child: MaterialApp(home: MainScreen()),
        ),
      );
      
      // Wait for initialization
      await tester.pumpAndSettle();
      
      // Verify spectrogram widget
      expect(find.byType(SpectrogramWidget), findsOneWidget);
      
      // Verify control panel
      expect(find.byType(ControlPanel), findsOneWidget);
      
      // Verify status bar
      expect(find.byType(StatusBar), findsOneWidget);
      
      // Verify navigation buttons
      expect(find.byIcon(Icons.settings), findsOneWidget);
      expect(find.byIcon(Icons.save_alt), findsOneWidget);
    });
    
    testWidgets('Settings navigation should work', (WidgetTester tester) async {
      await tester.pumpWidget(MelSpectrogramApp());
      
      // Wait for app to load
      await tester.pumpAndSettle();
      
      // Tap settings button
      await tester.tap(find.byIcon(Icons.settings));
      await tester.pumpAndSettle();
      
      // Verify settings screen is pushed
      expect(find.byType(SettingsScreen), findsOneWidget);
    });
    
    testWidgets('Export navigation should work', (WidgetTester tester) async {
      await tester.pumpWidget(MelSpectrogramApp());
      
      // Wait for app to load
      await tester.pumpAndSettle();
      
      // Tap export button
      await tester.tap(find.byIcon(Icons.save_alt));
      await tester.pumpAndSettle();
      
      // Verify export screen is pushed
      expect(find.byType(ExportScreen), findsOneWidget);
    });
  });
  
  group('Spectrogram Widget Tests', () {
    testWidgets('SpectrogramWidget should display correctly when active', (WidgetTester tester) async {
      final mockData = List.generate(64 * 256, (index) => index % 256);
      
      await tester.pumpWidget(
        MaterialApp(
          home: Scaffold(
            body: SpectrogramWidget(
              spectrogramData: mockData,
              isActive: true,
            ),
          ),
        ),
      );
      
      // Verify widget is displayed
      expect(find.byType(SpectrogramWidget), findsOneWidget);
      
      // Verify active state
      final widget = tester.widget<SpectrogramWidget>(find.byType(SpectrogramWidget));
      expect(widget.isActive, true);
      expect(widget.spectrogramData, equals(mockData));
    });
    
    testWidgets('SpectrogramWidget should handle empty data', (WidgetTester tester) async {
      await tester.pumpWidget(
        MaterialApp(
          home: Scaffold(
            body: SpectrogramWidget(
              spectrogramData: [],
              isActive: false,
            ),
          ),
        ),
      );
      
      // Should still render without crashing
      expect(find.byType(SpectrogramWidget), findsOneWidget);
    });
  });
  
  group('Control Panel Tests', () {
    testWidgets('ControlPanel should have all required buttons', (WidgetTester tester) async {
      await tester.pumpWidget(
        MaterialApp(
          home: Scaffold(
            body: ControlPanel(),
          ),
        ),
      );
      
      // Verify play/pause button
      expect(find.byIcon(Icons.play_arrow), findsOneWidget);
      
      // Verify stop button
      expect(find.byIcon(Icons.stop), findsOneWidget);
      
      // Verify reset button
      expect(find.byIcon(Icons.refresh), findsOneWidget);
    });
    
    testWidgets('ControlPanel buttons should be interactive', (WidgetTester tester) async {
      await tester.pumpWidget(
        MaterialApp(
          home: Scaffold(
            body: ControlPanel(),
          ),
        ),
      );
      
      // Tap play button
      await tester.tap(find.byIcon(Icons.play_arrow));
      await tester.pump();
      
      // Should change to pause icon when playing
      expect(find.byIcon(Icons.pause), findsOneWidget);
    });
  });
  
  group('Status Bar Tests', () {
    testWidgets('StatusBar should display performance metrics', (WidgetTester tester) async {
      await tester.pumpWidget(
        MaterialApp(
          home: Scaffold(
            body: StatusBar(),
          ),
        ),
      );
      
      // Verify status bar is present
      expect(find.byType(StatusBar), findsOneWidget);
      
      // Should contain performance indicators (exact text may vary)
      expect(find.byType(Text), findsWidgets);
    });
  });
  
  group('Service Tests', () {
    test('AudioService should initialize correctly', () {
      final audioService = AudioService();
      
      expect(audioService.isInitialized, false);
      expect(audioService.isRecording, false);
      expect(audioService.hasPermission, false);
    });
    
    test('SpectrogramService should initialize correctly', () {
      final spectrogramService = SpectrogramService();
      
      expect(spectrogramService.isProcessing, false);
      expect(spectrogramService.currentData, isEmpty);
      expect(spectrogramService.fps, 0.0);
    });
    
    test('AudioService permission handling', () async {
      final audioService = AudioService();
      
      // Mock permission request
      await audioService.requestPermission();
      
      // Should handle permission request without throwing
      expect(() async => await audioService.requestPermission(), returnsNormally);
    });
  });
  
  group('Integration Tests', () {
    testWidgets('Full app integration test', (WidgetTester tester) async {
      await tester.pumpWidget(MelSpectrogramApp());
      
      // Wait for full initialization
      await tester.pumpAndSettle(Duration(seconds: 2));
      
      // Verify all main components are present
      expect(find.byType(MainScreen), findsOneWidget);
      expect(find.byType(SpectrogramWidget), findsOneWidget);
      expect(find.byType(ControlPanel), findsOneWidget);
      expect(find.byType(StatusBar), findsOneWidget);
      
      // Test basic interaction
      await tester.tap(find.byIcon(Icons.play_arrow));
      await tester.pump();
      
      // Should start processing
      expect(find.byIcon(Icons.pause), findsOneWidget);
    });
    
    testWidgets('Theme and styling test', (WidgetTester tester) async {
      await tester.pumpWidget(MelSpectrogramApp());
      
      // Verify dark theme is applied
      final materialApp = tester.widget<MaterialApp>(find.byType(MaterialApp));
      expect(materialApp.theme?.scaffoldBackgroundColor, equals(Color(0xFF000000)));
      
      // Verify color scheme
      expect(materialApp.theme?.colorScheme?.primary, equals(Color(0xFF00ff88)));
      expect(materialApp.theme?.colorScheme?.secondary, equals(Color(0xFFff6600)));
    });
  });
}
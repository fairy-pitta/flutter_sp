import 'package:flutter_test/flutter_test.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:mel_spectrogram/main.dart';
import 'package:mel_spectrogram/audio_service.dart';

void main() {
  group('App smoke tests', () {
    testWidgets('App launches and shows title', (WidgetTester tester) async {
      // Wrap the app with the required providers
      await tester.pumpWidget(
        MultiProvider(
          providers: [
            ChangeNotifierProvider(create: (_) => AudioService()),
          ],
          child: const MelSpectrogramApp(),
        ),
      );
      
      // Wait for the app to settle
      await tester.pumpAndSettle();
      
      // Check if the app bar title is present
      expect(find.text('Mel Spectrogram'), findsOneWidget);
      
      // Check if the main screen is present
      expect(find.byType(Scaffold), findsOneWidget);
    });
  });
}
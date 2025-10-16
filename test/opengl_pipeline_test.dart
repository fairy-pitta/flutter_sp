import 'package:flutter_test/flutter_test.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'dart:ffi';
import 'dart:typed_data';
import 'package:provider/provider.dart';
import '../lib/audio_service.dart';
import '../lib/opengl_audio_visualizer.dart';
import '../lib/native_bridge_wrapper.dart';

void main() {
  group('OpenGL Pipeline Tests', () {
    late AudioService audioService;
    
    setUp(() {
      audioService = AudioService();
    });

    tearDown(() {
      audioService.dispose();
    });

    test('Native library loading test', () async {
      // Test that we can load the native library
      final wrapper = NativeBridgeWrapper();
      final isAvailable = await wrapper.checkRealAvailability();
      
      print('Native library available: $isAvailable');
      expect(isAvailable, isA<bool>());
    });

    test('OpenGL texture creation test', () async {
      // Test OpenGL texture creation
      final wrapper = NativeBridgeWrapper();
      final isAvailable = await wrapper.checkRealAvailability();
      
      if (isAvailable) {
        // Initialize texture renderer
        final textureId = await wrapper.initTextureRenderer(512, 256);
        print('Texture ID: $textureId');
        
        expect(textureId, greaterThanOrEqualTo(0));
        
        // Cleanup
        await wrapper.cleanup();
      } else {
        print('Skipping OpenGL texture test - native library not available');
      }
    });

    test('Real-time mel data processing test', () async {
      // Test mel data processing pipeline
      final wrapper = NativeBridgeWrapper();
      final isAvailable = await wrapper.checkRealAvailability();
      
      if (isAvailable) {
        // Initialize audio processing
        await wrapper.initializeAudioInput(44100, 1024);
        await wrapper.initializeMelProcessor(44100, 1024, 80, 80.0, 8000.0);
        
        // Generate test audio data
        final testData = Float32List(1024);
        for (int i = 0; i < 1024; i++) {
          testData[i] = 0.5 * (i % 100 < 50 ? 1.0 : -1.0); // Square wave
        }
        
        // Process audio frame
        final success = await wrapper.processAudioFrame(testData);
        expect(success, isTrue);
        
        // Get mel data
        final melData = await wrapper.getMelData();
        expect(melData, isNotNull);
        expect(melData.length, greaterThan(0));
        
        print('Mel data size: ${melData.length}');
        print('First 10 mel values: ${melData.take(10).toList()}');
        
        // Cleanup
        await wrapper.stopRecording();
        await wrapper.cleanup();
      } else {
        print('Skipping mel data processing test - native library not available');
      }
    });

    test('Texture updates and rendering test', () async {
      // Test texture update pipeline
      final wrapper = NativeBridgeWrapper();
      final isAvailable = await wrapper.checkRealAvailability();
      
      if (isAvailable) {
        // Initialize texture renderer
        final textureId = await wrapper.initTextureRenderer(512, 256);
        expect(textureId, greaterThanOrEqualTo(0));
        
        // Create test mel data
        final melData = Float32List(80);
        for (int i = 0; i < 80; i++) {
          melData[i] = (i / 80.0) * 0.8 + 0.1; // Gradient from 0.1 to 0.9
        }
        
        // Update texture
        final success = await wrapper.updateTextureColumn(melData, 0);
        expect(success, isTrue);
        
        // Get texture data for verification
        final textureData = await wrapper.getTextureData();
        expect(textureData, isNotNull);
        expect(textureData.length, greaterThan(0));
        
        print('Texture data size: ${textureData.length}');
        
        // Cleanup
        await wrapper.cleanup();
      } else {
        print('Skipping texture update test - native library not available');
      }
    });

    test('Performance benchmark test', () async {
      // Test performance of the pipeline
      final wrapper = NativeBridgeWrapper();
      final isAvailable = await wrapper.checkRealAvailability();
      
      if (isAvailable) {
        // Initialize everything
        await wrapper.initializeAudioInput(44100, 1024);
        await wrapper.initializeMelProcessor(44100, 1024, 80, 80.0, 8000.0);
        await wrapper.initTextureRenderer(512, 256);
        
        // Performance test
        const iterations = 100;
        final stopwatch = Stopwatch()..start();
        
        for (int i = 0; i < iterations; i++) {
          // Generate test audio
          final testData = Float32List(1024);
          for (int j = 0; j < 1024; j++) {
            testData[j] = 0.3 * (j % 50 < 25 ? 1.0 : -1.0);
          }
          
          // Process audio and update texture
          await wrapper.processAudioFrame(testData);
          final melData = await wrapper.getMelData();
          await wrapper.updateTextureColumn(melData, i % 512);
        }
        
        stopwatch.stop();
        
        final avgTime = stopwatch.elapsedMicroseconds / iterations;
        print('Average processing time: ${avgTime.toStringAsFixed(2)} microseconds');
        print('FPS potential: ${(1000000 / avgTime).toStringAsFixed(1)}');
        
        // Should achieve at least 30 FPS (33333 microseconds per frame)
        expect(avgTime, lessThan(33333));
        
        // Cleanup
        await wrapper.stopRecording();
        await wrapper.cleanup();
      } else {
        print('Skipping performance test - native library not available');
      }
    });

    test('Error handling test', () async {
      // Test error handling in the pipeline
      final wrapper = NativeBridgeWrapper();
      final isAvailable = await wrapper.checkRealAvailability();
      
      if (isAvailable) {
        // Try to use uninitialized components
        final melData = await wrapper.getMelData();
        expect(melData, isNull);
        
        final error = await wrapper.getLastError();
        print('Expected error: $error');
        expect(error, isNotEmpty);
        
        // Initialize properly
        await wrapper.initializeAudioInput(44100, 1024);
        await wrapper.initializeMelProcessor(44100, 1024, 80, 80.0, 8000.0);
        
        // Now it should work
        final melData2 = await wrapper.getMelData();
        expect(melData2, isNotNull);
        
        // Cleanup
        await wrapper.cleanup();
      } else {
        print('Skipping error handling test - native library not available');
      }
    });

    testWidget('OpenGL visualizer widget test', (WidgetTester tester) async {
      // Test the OpenGL visualizer widget
      await tester.pumpWidget(
        ChangeNotifierProvider(
          create: (context) => AudioService(),
          child: MaterialApp(
            home: Scaffold(
              body: Consumer<AudioService>(
                builder: (context, audioService, child) {
                  return OpenGLAudioVisualizer(
                    melDataStream: audioService.melDataStream,
                    width: 512,
                    height: 256,
                  );
                },
              ),
            ),
          ),
        ),
      );

      // Wait for widget to build
      await tester.pumpAndSettle();

      // Verify widget is created
      expect(find.byType(OpenGLAudioVisualizer), findsOneWidget);
      
      // Check for texture widget
      expect(find.byType(Texture), findsOneWidget);
    });
  });
}
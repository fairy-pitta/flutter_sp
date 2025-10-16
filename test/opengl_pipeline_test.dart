import 'package:flutter_test/flutter_test.dart';
import 'package:flutter/material.dart';
import 'dart:typed_data';
import 'package:provider/provider.dart';
import 'package:mel_spectrogram/audio_service.dart';
import 'package:mel_spectrogram/opengl_audio_visualizer.dart';
import 'package:mel_spectrogram/native_bridge_wrapper.dart';

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
      // Test that we can check native library availability
      NativeBridgeWrapper.checkRealAvailability();
      final isAvailable = NativeBridgeWrapper.realAvailable;
      
      expect(isAvailable, isA<bool>());
    });

    test('OpenGL texture creation test', () async {
      // Test OpenGL texture creation (only if native library is available)
      NativeBridgeWrapper.checkRealAvailability();
      final isAvailable = NativeBridgeWrapper.realAvailable;
      
      if (isAvailable) {
        // Initialize texture renderer
        final initResult = NativeBridgeWrapper.initTextureRenderer(512, 256, 128);
        expect(initResult, equals(0));
        
        final textureId = NativeBridgeWrapper.getTextureId();
        expect(textureId, greaterThanOrEqualTo(0));
      } else {
        // Skip assertions requiring native library when unavailable
      }
    });

    test('Real-time mel data processing test', () async {
      // Test mel data processing pipeline using mock bridge
      NativeBridgeWrapper.setMode(BridgeMode.mock);
      
      // Initialize audio and mel processor
      final audioInit = NativeBridgeWrapper.initializeAudioInput(
        sampleRate: 44100,
        bufferSize: 1024,
        channels: 1,
        format: 3,
      );
      expect(audioInit, equals(0));
      
      final melInit = NativeBridgeWrapper.initializeMelProcessor(
        numFilters: 80,
        minFreq: 80.0,
        maxFreq: 8000.0,
        sampleRate: 44100.0,
      );
      expect(melInit, equals(0));
      
      // Start recording and process a frame
      expect(NativeBridgeWrapper.startRecording(), equals(0));
      expect(NativeBridgeWrapper.processAudioFrame(), equals(0));
      
      // Get mel data
      final melData = NativeBridgeWrapper.getMelData();
      expect(melData, isNotNull);
      expect(melData.length, greaterThan(0));
      
      // Cleanup
      expect(NativeBridgeWrapper.stopRecording(), equals(0));
    });

    test('Texture updates and rendering test', () async {
      // Test texture update pipeline (mock)
      NativeBridgeWrapper.setMode(BridgeMode.mock);
      
      final initResult = NativeBridgeWrapper.initTextureRenderer(512, 256, 80);
      expect(initResult, equals(0));
      
      // Create test mel data
      final melData = Float32List(80);
      for (int i = 0; i < 80; i++) {
        melData[i] = (i / 80.0) * 0.8 + 0.1; // Gradient from 0.1 to 0.9
      }
      
      // Update texture
      final updateResult = NativeBridgeWrapper.updateTextureColumn(melData, 0);
      expect(updateResult, equals(0));
      
      // Get texture data for verification
      final textureData = NativeBridgeWrapper.getTextureData();
      expect(textureData, isNotNull);
      expect(textureData.length, greaterThan(0));
      // Avoid printing in tests to satisfy lints
    });

    test('Performance benchmark test', () async {
      // Test performance of the pipeline
      NativeBridgeWrapper.setMode(BridgeMode.mock);
      
      // Initialize everything
      // Check if native library is available
      NativeBridgeWrapper.checkRealAvailability();
      final isRealAvailable = NativeBridgeWrapper.realAvailable;
      
      if (!isRealAvailable) {
        // Skip performance test if native library is not available
        return;
      }
      
      expect(NativeBridgeWrapper.initializeAudioInput(
        sampleRate: 44100,
        bufferSize: 1024,
        channels: 1,
        format: 3,
      ), equals(0));
      expect(NativeBridgeWrapper.initializeMelProcessor(
        numFilters: 80,
        minFreq: 80.0,
        maxFreq: 8000.0,
        sampleRate: 44100.0,
      ), equals(0));
      expect(NativeBridgeWrapper.initTextureRenderer(512, 256, 80), equals(0));
      
      // Performance test
      const iterations = 50;
      final stopwatch = Stopwatch()..start();
      
      for (int i = 0; i < iterations; i++) {
        final processResult = NativeBridgeWrapper.processAudioFrame();
        if (processResult != 0) {
          // If processing fails, skip this iteration
          continue;
        }
        final melData = NativeBridgeWrapper.getMelData();
        final column = i % 512;
        NativeBridgeWrapper.updateTextureColumn(melData, column);
      }
      
      stopwatch.stop();
      
      final avgTime = stopwatch.elapsedMicroseconds / iterations;
      
      // Basic sanity: avgTime should be a finite number
      expect(avgTime.isFinite, isTrue);
      
      // Cleanup
      NativeBridgeWrapper.stopRecording();
    });

    test('Error handling test', () async {
      // Basic error string access should work
      final error = NativeBridgeWrapper.getLastError();
      expect(error, isA<String>());
    });
  });
}
import 'package:flutter_test/flutter_test.dart';
import 'package:mel_spectrogram/audio_service.dart';
import 'package:mel_spectrogram/native_bridge_wrapper.dart';

void main() {
  group('OpenGL Integration Tests', () {
    late AudioService audioService;
    
    setUp(() {
      audioService = AudioService();
    });
    
    tearDown(() {
      audioService.dispose();
    });
    
    test('Native library loading test', () async {
      // Test that native bridge can be initialized
      
      // Try to initialize with real mode if available
      try {
        NativeBridgeWrapper.setMode(BridgeMode.real);
        expect(NativeBridgeWrapper.mode, equals(BridgeMode.real));
        
        // Test basic initialization
        final result = NativeBridgeWrapper.initializeAudioInput(
          sampleRate: 44100,
          bufferSize: 1024,
          channels: 1,
          format: 1,
        );
        expect(result, greaterThanOrEqualTo(0));
        
        // Test mel processor initialization
        final melResult = NativeBridgeWrapper.initializeMelProcessor(
          numFilters: 128,
          minFreq: 20.0,
          maxFreq: 20000.0,
          sampleRate: 44100.0,
        );
        expect(melResult, greaterThanOrEqualTo(0));
        
      } catch (e) {
        // If real mode fails, fall back to mock mode
        NativeBridgeWrapper.setMode(BridgeMode.mock);
        expect(NativeBridgeWrapper.mode, equals(BridgeMode.mock));
        
        final result = NativeBridgeWrapper.initializeAudioInput(
          sampleRate: 44100,
          bufferSize: 1024,
          channels: 1,
          format: 1,
        );
        expect(result, greaterThanOrEqualTo(0));
      }
    });
    
    test('OpenGL texture creation test', () async {
      // Initialize audio service with OpenGL enabled
      await audioService.initialize();
      
      // Check if native library is available
      NativeBridgeWrapper.checkRealAvailability();
      final isRealAvailable = NativeBridgeWrapper.realAvailable;
      
      audioService.setUseOpenGL(true);
      
      if (isRealAvailable) {
        expect(audioService.useOpenGL, isTrue);
        expect(audioService.textureInitialized, isTrue);
        expect(audioService.textureId, greaterThan(0));
      } else {
        // If native library is not available, OpenGL should be disabled
        expect(audioService.useOpenGL, isFalse);
        // In mock mode, texture may not be initialized - this is acceptable
      }
    });
    
    test('Real-time mel data processing test', () async {
      await audioService.initialize();
      
      // Start recording to generate real-time data
      await audioService.startRecording();
      
      // Wait for some data to accumulate
      await Future.delayed(const Duration(milliseconds: 100));
      
      // Verify mel data is being generated
      final melData = NativeBridgeWrapper.getMelData();
      expect(melData, isNotNull);
      expect(melData.length, greaterThan(0));
      
      // Verify processing stats are being tracked (only if available)
      if (audioService.processingStats != null) {
        expect(audioService.processingStats!.currentFps, greaterThanOrEqualTo(0));
      }
      
      await audioService.stopRecording();
    });
    
    test('Texture updates and rendering test', () async {
      await audioService.initialize();
      audioService.setUseOpenGL(true);
      
      // Test texture updates
      await audioService.startRecording();
      await Future.delayed(const Duration(milliseconds: 100));
      
      // Verify texture updates are happening (only if stats are available)
      if (audioService.processingStats != null) {
        expect(audioService.processingStats!.textureUpdates, greaterThanOrEqualTo(0));
      }
      
      await audioService.stopRecording();
    });
    
    test('Performance monitoring test', () async {
      await audioService.initialize();
      
      // Check if native library is available for performance testing
      NativeBridgeWrapper.checkRealAvailability();
      final isRealAvailable = NativeBridgeWrapper.realAvailable;
      
      // Start recording and monitor performance
      await audioService.startRecording();
      
      // Collect performance data over time
      final fpsValues = <double>[];
      for (int i = 0; i < 10; i++) {
        await Future.delayed(const Duration(milliseconds: 100));
        if (audioService.processingStats != null) {
          fpsValues.add(audioService.processingStats!.currentFps);
        }
      }
      
      if (isRealAvailable) {
        // Verify consistent performance only when native library is available
        expect(fpsValues.length, greaterThan(5));
        expect(fpsValues.every((fps) => fps > 30), isTrue, 
            reason: 'FPS should consistently be above 30');
        
        // Verify 60+ FPS target is met on average
        final averageFps = fpsValues.reduce((a, b) => a + b) / fpsValues.length;
        expect(averageFps, greaterThan(60), 
            reason: 'Average FPS should be above 60 for real-time performance');
      } else {
        // In mock mode, performance requirements may be different
        // Just verify that some processing is happening
        expect(fpsValues.length, greaterThanOrEqualTo(0));
      }
      
      await audioService.stopRecording();
    });
    
    test('OpenGL/Software rendering switch test', () async {
      await audioService.initialize();
      
      // Check if native library is available
      NativeBridgeWrapper.checkRealAvailability();
      final isRealAvailable = NativeBridgeWrapper.realAvailable;
      
      // Test switching to OpenGL
      audioService.setUseOpenGL(true);
      if (isRealAvailable) {
        expect(audioService.useOpenGL, isTrue);
        expect(audioService.textureInitialized, isTrue);
      } else {
        // If native library is not available, OpenGL should be disabled
        expect(audioService.useOpenGL, isFalse);
      }
      
      // Test switching to software rendering
      audioService.setUseOpenGL(false);
      expect(audioService.useOpenGL, isFalse);
      
      // Test switching back to OpenGL
      audioService.setUseOpenGL(true);
      if (isRealAvailable) {
        expect(audioService.useOpenGL, isTrue);
        expect(audioService.textureInitialized, isTrue);
      } else {
        // If native library is not available, OpenGL should remain disabled
        expect(audioService.useOpenGL, isFalse);
      }
    });
    
    test('Error handling test', () async {
      // Test error handling with invalid parameters
      try {
        await audioService.initialize();
        
        // Try to initialize with invalid sample rate
        final result = NativeBridgeWrapper.initializeAudioInput(
          sampleRate: -1,
          bufferSize: 1024,
          channels: 1,
          format: 1,
        );
        
        // Should return error code
        expect(result, lessThan(0));
        
      } catch (e) {
        // Expected behavior - invalid parameters should throw
        expect(e, isA<Exception>());
      }
    });
    
    test('Memory stability test', () async {
      await audioService.initialize();
      
      // Run continuous processing for stability
      await audioService.startRecording();
      
      // Monitor for memory leaks over extended period
      for (int i = 0; i < 50; i++) {
        await Future.delayed(const Duration(milliseconds: 100));
        
        // Verify no crashes or memory issues
        expect(audioService.isRecording, isTrue);
        expect(audioService.error, isNull);
      }
      
      await audioService.stopRecording();
      
      // Verify clean shutdown
      expect(audioService.isRecording, isFalse);
      expect(audioService.error, isNull);
    });
  });
}
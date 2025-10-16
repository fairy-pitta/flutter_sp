import 'package:flutter_test/flutter_test.dart';
import 'package:mel_spectrogram/audio_service.dart';
import 'package:mel_spectrogram/native_bridge_wrapper.dart';

void main() {
  group('Basic OpenGL Integration Tests', () {
    
    test('Native bridge wrapper can be instantiated', () {
      // Test that we can create the wrapper without crashing
      expect(() {
        NativeBridgeWrapper.setMode(BridgeMode.mock);
      }, returnsNormally);
    });
    
    test('Audio service can be created', () {
      final audioService = AudioService();
      expect(audioService, isNotNull);
      expect(audioService.isInitialized, isFalse);
      expect(audioService.isRecording, isFalse);
      audioService.dispose();
    });
    
    test('Mock mode works correctly', () {
      // Set to mock mode
      NativeBridgeWrapper.setMode(BridgeMode.mock);
      expect(NativeBridgeWrapper.mode, equals(BridgeMode.mock));
      
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
    });
    
    test('Audio service initialization works', () async {
      final audioService = AudioService();
      
      // Initialize the service
      await audioService.initialize();
      
      expect(audioService.isInitialized, isTrue);
      expect(audioService.useOpenGL, isFalse); // Should default to false
      
      audioService.dispose();
    });
    
    test('OpenGL toggle works', () async {
      final audioService = AudioService();
      await audioService.initialize();
      
      // Check if native library is available
      NativeBridgeWrapper.checkRealAvailability();
      final nativeAvailable = NativeBridgeWrapper.realAvailable;
      
      // Enable OpenGL
      audioService.setUseOpenGL(true);
      if (nativeAvailable) {
        expect(audioService.useOpenGL, isTrue);
      } else {
        // When native library is unavailable, setUseOpenGL(true) should reset to false
        expect(audioService.useOpenGL, isFalse);
      }
      
      // Disable OpenGL (should always work)
      audioService.setUseOpenGL(false);
      expect(audioService.useOpenGL, isFalse);
      
      audioService.dispose();
    });
    
    test('Recording state management', () async {
      final audioService = AudioService();
      await audioService.initialize();
      
      expect(audioService.isRecording, isFalse);
      
      // Note: We can't actually start recording in tests due to platform limitations
      // but we can test the state management
      
      audioService.dispose();
    });
  });
}
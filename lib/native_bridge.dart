import 'dart:math' as math;
import 'dart:typed_data';

// Mock implementation for testing without native library
class NativeBridge {
  static bool _initialized = false;
  static bool _recording = false;
  static int _sampleRate = 44100;
  static int _bufferSize = 1024;
  static int _numFilters = 128;
  static double _minFreq = 20.0;
  static double _maxFreq = 20000.0;
  static double _phase = 0.0;
  
  static int initializeAudioInput({
    required int sampleRate,
    required int bufferSize,
    required int channels,
    required int format,
  }) {
    _sampleRate = sampleRate;
    _bufferSize = bufferSize;
    _initialized = true;
    return 0; // Success
  }
  
  static int initializeMelProcessor({
    required int numFilters,
    required double minFreq,
    required double maxFreq,
    required double sampleRate,
  }) {
    _numFilters = numFilters;
    _minFreq = minFreq;
    _maxFreq = maxFreq;
    return 0; // Success
  }
  
  static int startRecording() {
    if (!_initialized) return -1;
    _recording = true;
    return 0; // Success
  }
  
  static int stopRecording() {
    _recording = false;
    return 0; // Success
  }
  
  static int processAudioFrame() {
    if (!_recording) return -1;
    return 0; // Success
  }
  
  static Float32List getMelData() {
    // Generate mock mel spectrogram data
    final melData = Float32List(_numFilters);
    final time = DateTime.now().millisecondsSinceEpoch / 1000.0;
    
    for (int i = 0; i < _numFilters; i++) {
      final freq = _minFreq + (i / (_numFilters - 1)) * (_maxFreq - _minFreq);
      final normalizedFreq = freq / (_maxFreq - _minFreq);
      
      // Generate synthetic audio signal with multiple frequencies
      double amplitude = 0.0;
      
      // Add some harmonics
      amplitude += math.sin(2 * math.pi * 440 * time + _phase) * math.exp(-normalizedFreq * 2);
      amplitude += 0.5 * math.sin(2 * math.pi * 880 * time + _phase * 1.5) * math.exp(-(normalizedFreq - 0.5).abs() * 3);
      amplitude += 0.3 * math.sin(2 * math.pi * 1320 * time + _phase * 2.0) * math.exp(-(normalizedFreq - 0.7).abs() * 4);
      
      // Add some noise
      amplitude += (math.Random().nextDouble() - 0.5) * 0.1;
      
      // Convert to dB scale
      final power = amplitude * amplitude;
      final logPower = 10 * math.log(math.max(power, 1e-10)) / math.ln10;
      
      // Normalize to 0-1 range
      melData[i] = math.max(0.0, math.min(1.0, (logPower + 80) / 80));
    }
    
    _phase += 0.1;
    return melData;
  }
  
  static int getMelDataSize() {
    return _numFilters;
  }
  
  static String getLastError() {
    return 'No error';
  }
  
  // OpenGL Texture Renderer Functions
  static int initTextureRenderer(int width, int height, int numMelBands) {
    // Mock implementation - return success
    return 0;
  }
  
  static int updateTextureColumn(Float32List melData) {
    // Mock implementation - return success
    return 0;
  }
  
  static int getTextureId() {
    // Mock implementation - return a fake texture ID
    return 12345;
  }
  
  static String getTextureData() {
    // Mock implementation
    return 'Mock texture data';
  }
  
  static int setTextureColorMap(int colorMapType) {
    // Mock implementation - return success
    return 0;
  }
  
  static int setTextureMinMax(double minValue, double maxValue) {
    // Mock implementation - return success
    return 0;
  }
}
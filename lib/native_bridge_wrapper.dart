import 'dart:typed_data';
import 'native_bridge.dart' as mock;
import 'native_bridge_real.dart' as real;

enum BridgeMode { mock, real }

class NativeBridgeWrapper {
  static BridgeMode _mode = BridgeMode.mock;
  static bool _realAvailable = false;
  
  static void setMode(BridgeMode mode) {
    _mode = mode;
  }
  
  static BridgeMode get mode => _mode;
  
  static bool get realAvailable => _realAvailable;
  
  static void checkRealAvailability() {
    try {
      // Try to load the real library
      real.NativeBridgeReal.initializeAudioInput(
        sampleRate: 44100,
        bufferSize: 1024,
        channels: 1,
        format: 3, // FLOAT32_LE
      );
      _realAvailable = true;
    } catch (e) {
      _realAvailable = false;
    }
  }
  
  static int initializeAudioInput({
    required int sampleRate,
    required int bufferSize,
    required int channels,
    required int format,
  }) {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.initializeAudioInput(
        sampleRate: sampleRate,
        bufferSize: bufferSize,
        channels: channels,
        format: format,
      );
    } else {
      return mock.NativeBridge.initializeAudioInput(
        sampleRate: sampleRate,
        bufferSize: bufferSize,
        channels: channels,
        format: format,
      );
    }
  }
  
  static int initializeMelProcessor({
    required int numFilters,
    required double minFreq,
    required double maxFreq,
    required double sampleRate,
  }) {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.initializeMelProcessor(
        numFilters: numFilters,
        minFreq: minFreq,
        maxFreq: maxFreq,
        sampleRate: sampleRate,
      );
    } else {
      return mock.NativeBridge.initializeMelProcessor(
        numFilters: numFilters,
        minFreq: minFreq,
        maxFreq: maxFreq,
        sampleRate: sampleRate,
      );
    }
  }
  
  static int startRecording() {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.startRecording();
    } else {
      return mock.NativeBridge.startRecording();
    }
  }
  
  static int stopRecording() {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.stopRecording();
    } else {
      return mock.NativeBridge.stopRecording();
    }
  }
  
  static int processAudioFrame() {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.processAudioFrame();
    } else {
      return mock.NativeBridge.processAudioFrame();
    }
  }
  
  static Float32List getMelData() {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.getMelData();
    } else {
      return mock.NativeBridge.getMelData();
    }
  }
  
  static int getMelDataSize() {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.getMelDataSize();
    } else {
      return mock.NativeBridge.getMelDataSize();
    }
  }
  
  static String getLastError() {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.getLastError();
    } else {
      return mock.NativeBridge.getLastError();
    }
  }
  
  // OpenGL Texture Renderer Functions
  static int initTextureRenderer(int width, int height, int numMelBands) {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.initTextureRenderer(width, height, numMelBands);
    } else {
      return mock.NativeBridge.initTextureRenderer(width, height, numMelBands);
    }
  }
  
  static int updateTextureColumn(Float32List melData, [int column = 0]) {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.updateTextureColumn(melData);
    } else {
      return mock.NativeBridge.updateTextureColumn(melData, column);
    }
  }
  
  static Float32List? getAudioFrame() {
      if (_mode == BridgeMode.real && _realAvailable) {
        return real.NativeBridgeReal.getAudioFrame();
      } else {
        return mock.NativeBridge.getAudioFrame();
      }
    }
  
  static int getTextureId() {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.getTextureId();
    } else {
      return mock.NativeBridge.getTextureId();
    }
  }
  
  static String getTextureData() {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.getTextureData();
    } else {
      return mock.NativeBridge.getTextureData();
    }
  }
  
  static int setTextureColorMap(int colorMapType) {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.setTextureColorMap(colorMapType);
    } else {
      return mock.NativeBridge.setTextureColorMap(colorMapType);
    }
  }
  
  static int setTextureMinMax(double minValue, double maxValue) {
    if (_mode == BridgeMode.real && _realAvailable) {
      return real.NativeBridgeReal.setTextureMinMax(minValue, maxValue);
    } else {
      return mock.NativeBridge.setTextureMinMax(minValue, maxValue);
    }
  }
}
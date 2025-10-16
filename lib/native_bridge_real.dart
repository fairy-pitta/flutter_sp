import 'dart:ffi';
import 'dart:io';
import 'dart:typed_data';
import 'dart:math' as math;
import 'package:ffi/ffi.dart';

// FFI bindings for the native library
final class AudioConfigNative extends Struct {
  @Int32()
  external int sampleRate;
  
  @Int32()
  external int bufferSize;
  
  @Int32()
  external int numChannels;
  
  @Int32()
  external int format;
  
  @Int32()
  external int platform;
}

final class MelConfigNative extends Struct {
  @Int32()
  external int sampleRate;
  
  @Int32()
  external int frameSize;
  
  @Int32()
  external int hopSize;
  
  @Int32()
  external int numMelBands;
  
  @Float()
  external double minFreq;
  
  @Float()
  external double maxFreq;
}

typedef InitAudioInputFunc = Int32 Function(Pointer<AudioConfigNative> config);
typedef InitAudioInput = int Function(Pointer<AudioConfigNative> config);

typedef StartRecordingFunc = Int32 Function();
typedef StartRecording = int Function();

typedef StopRecordingFunc = Int32 Function();
typedef StopRecording = int Function();

typedef InitMelProcessorFunc = Int32 Function(Pointer<MelConfigNative> config);
typedef InitMelProcessor = int Function(Pointer<MelConfigNative> config);

typedef ProcessAudioFrameFunc = Int32 Function(Pointer<Int16> inputBuffer, Int32 bufferSize, 
                                               Pointer<Float> outputBuffer, Int32 outputSize);
typedef ProcessAudioFrame = int Function(Pointer<Int16> inputBuffer, int bufferSize, 
                                         Pointer<Float> outputBuffer, int outputSize);

typedef GetMelDataSizeFunc = Int32 Function();
typedef GetMelDataSize = int Function();

typedef GetErrorMessageFunc = Pointer<Utf8> Function();
typedef GetErrorMessage = Pointer<Utf8> Function();

typedef InitTextureRendererFunc = Int32 Function(Int32 width, Int32 height, Int32 numMelBands);
typedef InitTextureRenderer = int Function(int width, int height, int numMelBands);

typedef UpdateTextureColumnFunc = Int32 Function(Pointer<Float> melData, Int32 dataSize);
typedef UpdateTextureColumn = int Function(Pointer<Float> melData, int dataSize);

typedef GetTextureIdFunc = Uint32 Function();
typedef GetTextureId = int Function();

typedef GetTextureDataFunc = Int32 Function(Pointer<Uint8> buffer, Int32 bufferSize);
typedef GetTextureData = int Function(Pointer<Uint8> buffer, int bufferSize);

typedef SetTextureColorMapFunc = Int32 Function(Int32 colorMapType);
typedef SetTextureColorMap = int Function(int colorMapType);

typedef SetTextureMinMaxFunc = Int32 Function(Double minValue, Double maxValue);
typedef SetTextureMinMax = int Function(double minValue, double maxValue);

class NativeBridgeReal {
  static DynamicLibrary? _lib;
  static bool _initialized = false;
  
  // Function pointers
  static InitAudioInput? _initAudioInput;
  static StartRecording? _startRecording;
  static StopRecording? _stopRecording;
  static InitMelProcessor? _initMelProcessor;
  static ProcessAudioFrame? _processAudioFrame;
  static GetMelDataSize? _getMelDataSize;
  static GetErrorMessage? _getErrorMessage;
  static InitTextureRenderer? _initTextureRenderer;
  static UpdateTextureColumn? _updateTextureColumn;
  static GetTextureId? _getTextureId;
  static GetTextureData? _getTextureData;
  static SetTextureColorMap? _setTextureColorMap;
  static SetTextureMinMax? _setTextureMinMax;
  
  static void _loadLibrary() {
    if (_lib != null) return;
    
    try {
      if (Platform.isMacOS) {
        _lib = DynamicLibrary.open('libflutter_sp_native.dylib');
      } else if (Platform.isLinux) {
        _lib = DynamicLibrary.open('libflutter_sp_native.so');
      } else if (Platform.isWindows) {
        _lib = DynamicLibrary.open('flutter_sp_native.dll');
      } else {
        throw UnsupportedError('Unsupported platform');
      }
      
      _bindFunctions();
      _initialized = true;
    } catch (e) {
      print('Failed to load native library: $e');
      throw Exception('Failed to load native library: $e');
    }
  }
  
  static void _bindFunctions() {
    _initAudioInput = _lib!.lookupFunction<InitAudioInputFunc, InitAudioInput>('init_audio_input');
    _startRecording = _lib!.lookupFunction<StartRecordingFunc, StartRecording>('start_recording');
    _stopRecording = _lib!.lookupFunction<StopRecordingFunc, StopRecording>('stop_recording');
    _initMelProcessor = _lib!.lookupFunction<InitMelProcessorFunc, InitMelProcessor>('init_mel_processor');
    _processAudioFrame = _lib!.lookupFunction<ProcessAudioFrameFunc, ProcessAudioFrame>('process_audio_frame');
    _getMelDataSize = _lib!.lookupFunction<GetMelDataSizeFunc, GetMelDataSize>('get_mel_data_size');
    _getErrorMessage = _lib!.lookupFunction<GetErrorMessageFunc, GetErrorMessage>('get_error_message');
    _initTextureRenderer = _lib!.lookupFunction<InitTextureRendererFunc, InitTextureRenderer>('init_texture_renderer');
    _updateTextureColumn = _lib!.lookupFunction<UpdateTextureColumnFunc, UpdateTextureColumn>('update_texture_column');
    _getTextureId = _lib!.lookupFunction<GetTextureIdFunc, GetTextureId>('get_texture_id');
    _getTextureData = _lib!.lookupFunction<GetTextureDataFunc, GetTextureData>('get_texture_data');
    _setTextureColorMap = _lib!.lookupFunction<SetTextureColorMapFunc, SetTextureColorMap>('set_texture_color_map');
    _setTextureMinMax = _lib!.lookupFunction<SetTextureMinMaxFunc, SetTextureMinMax>('set_texture_min_max');
  }
  
  static int initializeAudioInput({
    required int sampleRate,
    required int bufferSize,
    required int channels,
    required int format,
  }) {
    if (!_initialized) _loadLibrary();
    
    final config = malloc<AudioConfigNative>();
    config.ref.sampleRate = sampleRate;
    config.ref.bufferSize = bufferSize;
    config.ref.numChannels = channels;
    config.ref.format = format;
    config.ref.platform = 2; // MOCK platform
    
    final result = _initAudioInput!(config);
    malloc.free(config);
    
    return result;
  }
  
  static int initializeMelProcessor({
    required int numFilters,
    required double minFreq,
    required double maxFreq,
    required double sampleRate,
  }) {
    if (!_initialized) _loadLibrary();
    
    final config = malloc<MelConfigNative>();
    config.ref.sampleRate = sampleRate.toInt();
    config.ref.frameSize = 1024;
    config.ref.hopSize = 512;
    config.ref.numMelBands = numFilters;
    config.ref.minFreq = minFreq;
    config.ref.maxFreq = maxFreq;
    
    final result = _initMelProcessor!(config);
    malloc.free(config);
    
    return result;
  }
  
  static int startRecording() {
    if (!_initialized) return -1;
    return _startRecording!();
  }
  
  static int stopRecording() {
    if (!_initialized) return -1;
    return _stopRecording!();
  }
  
  static int processAudioFrame() {
    if (!_initialized) return -1;
    
    // Generate mock audio data for now
    final mockData = malloc<Int16>(1024);
    final outputBuffer = malloc<Float>(256);
    
    for (int i = 0; i < 1024; i++) {
      mockData[i] = (math.sin(i * 0.1) * 32767).toInt();
    }
    
    final result = _processAudioFrame!(mockData, 1024, outputBuffer, 256);
    
    malloc.free(mockData);
    malloc.free(outputBuffer);
    
    return result;
  }
  
  static Float32List getMelData() {
    if (!_initialized) return Float32List(0);
    
    final size = _getMelDataSize!();
    if (size <= 0) return Float32List(0);
    
    // For now, return mock data until we implement proper data retrieval
    final mockData = Float32List(size);
    for (int i = 0; i < size; i++) {
      mockData[i] = math.Random().nextDouble();
    }
    return mockData;
  }
  
  static int getMelDataSize() {
    if (!_initialized) return 0;
    return _getMelDataSize!();
  }
  
  static String getLastError() {
    if (!_initialized) return 'Native library not initialized';
    
    final errorPtr = _getErrorMessage!();
    return errorPtr.cast<Utf8>().toDartString();
  }
  
  // OpenGL Texture Renderer Functions
  static int initTextureRenderer(int width, int height, int numMelBands) {
    if (!_initialized) return -1;
    return _initTextureRenderer!(width, height, numMelBands);
  }
  
  static int updateTextureColumn(Float32List melData) {
    if (!_initialized) return -1;
    
    final dataPtr = malloc<Float>(melData.length);
    for (int i = 0; i < melData.length; i++) {
      dataPtr[i] = melData[i];
    }
    
    final result = _updateTextureColumn!(dataPtr, melData.length);
    malloc.free(dataPtr);
    
    return result;
  }
  
  static int getTextureId() {
    if (!_initialized) return 0;
    return _getTextureId!();
  }
  
  static String getTextureData() {
    if (!_initialized) return 'Native library not initialized';
    
    // For now, return mock data
    return 'Real texture data from native library';
  }
  
  static int setTextureColorMap(int colorMapType) {
    if (!_initialized) return -1;
    return _setTextureColorMap!(colorMapType);
  }
  
  static int setTextureMinMax(double minValue, double maxValue) {
    if (!_initialized) return -1;
    return _setTextureMinMax!(minValue, maxValue);
  }
  
  static Float32List? getAudioFrame() {
    if (!_initialized) return null;
    
    // Generate mock audio frame data for now
    final frame = Float32List(1024);
    for (int i = 0; i < frame.length; i++) {
      frame[i] = math.sin(i * 0.1) * 0.5 + math.Random().nextDouble() * 0.1;
    }
    return frame;
  }
}
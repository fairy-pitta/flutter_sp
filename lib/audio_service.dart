import 'dart:async';
import 'dart:typed_data';
import 'dart:math' as math;
import 'package:flutter/foundation.dart';
import 'package:logging/logging.dart';
import 'native_bridge_wrapper.dart';

enum AudioFormat {
  s16le(0),
  s24le(1),
  s32le(2),
  float32le(3);
  
  const AudioFormat(this.value);
  final int value;
}

enum AudioState {
  idle,
  initializing,
  ready,
  recording,
  error,
  disposed,
}

class AudioConfig {
  final int sampleRate;
  final int bufferSize;
  final int channels;
  final AudioFormat format;
  
  const AudioConfig({
    this.sampleRate = 44100,
    this.bufferSize = 1024,
    this.channels = 1,
    this.format = AudioFormat.float32le,
  });
}

class MelConfig {
  final int numFilters;
  final double minFreq;
  final double maxFreq;
  final double sampleRate;
  
  const MelConfig({
    this.numFilters = 128,
    this.minFreq = 20.0,
    this.maxFreq = 20000.0,
    this.sampleRate = 44100.0,
  });
}

class AudioProcessingStats {
  int framesProcessed;
  int framesDropped;
  double averageProcessingTime;
  double currentFps;
  double audioLevel;
  int textureUpdates;
  double lastFrameTime;
  
  AudioProcessingStats({
    this.framesProcessed = 0,
    this.framesDropped = 0,
    this.averageProcessingTime = 0.0,
    this.currentFps = 0.0,
    this.audioLevel = -60.0,
    this.textureUpdates = 0,
    this.lastFrameTime = 0.0,
  });
}

class AudioService extends ChangeNotifier {
  static final Logger _logger = Logger('AudioService');
  
  AudioState _state = AudioState.idle;
  AudioConfig _config;
  MelConfig _melConfig;
  AudioProcessingStats _stats = AudioProcessingStats();
  String? _error;
  double? _audioLevel;
  
  // Real audio processing
  Timer? _processingTimer;
  Float32List? _currentAudioFrame;
  int _frameCounter = 0;
  
  final StreamController<Float32List> _melStreamController = StreamController<Float32List>.broadcast();
  final StreamController<AudioProcessingStats> _statsStreamController = StreamController<AudioProcessingStats>.broadcast();
  
  // OpenGL texture renderer
  bool _useOpenGL = true;
  int? _textureId;
  bool _textureInitialized = false;
  
  AudioService({
    AudioConfig? audioConfig,
    MelConfig? melConfig,
  }) : _config = audioConfig ?? const AudioConfig(),
       _melConfig = melConfig ?? const MelConfig() {
    _initializeLogging();
  }
  
  AudioState get state => _state;
  AudioConfig get config => _config;
  MelConfig get melConfig => _melConfig;
  AudioProcessingStats get stats => _stats;
  Stream<Float32List> get melDataStream => _melStreamController.stream;
  Stream<AudioProcessingStats> get statsStream => _statsStreamController.stream;
  bool get isInitialized => _state != AudioState.idle && _state != AudioState.error;
  bool get isRecording => _state == AudioState.recording;
  String? get error => _error;
  double? get audioLevel => _audioLevel;
  AudioProcessingStats? get processingStats => _stats;
  
  void _initializeLogging() {
    Logger.root.level = Level.ALL;
    Logger.root.onRecord.listen((record) {
      if (kDebugMode) {
        print('${record.level.name}: ${record.time}: ${record.message}');
      }
    });
  }
  
  Future<bool> initialize() async {
    if (_state != AudioState.idle && _state != AudioState.error) {
      _logger.warning('Cannot initialize in state: $_state');
      return false;
    }
    
    _state = AudioState.initializing;
    notifyListeners();
    
    try {
      _logger.info('Initializing audio input with config: sampleRate=${_config.sampleRate}, '
          'bufferSize=${_config.bufferSize}, channels=${_config.channels}');
      
      // Initialize audio input
      final audioResult = NativeBridgeWrapper.initializeAudioInput(
        sampleRate: _config.sampleRate,
        bufferSize: _config.bufferSize,
        channels: _config.channels,
        format: _config.format.value,
      );
      
      if (audioResult != 0) {
        throw Exception('Failed to initialize audio input: ${NativeBridgeWrapper.getLastError()}');
      }
      
      _logger.info('Initializing mel processor with config: numFilters=${_melConfig.numFilters}, '
          'minFreq=${_melConfig.minFreq}, maxFreq=${_melConfig.maxFreq}');
      
      // Initialize mel processor
      final melResult = NativeBridgeWrapper.initializeMelProcessor(
        numFilters: _melConfig.numFilters,
        minFreq: _melConfig.minFreq,
        maxFreq: _melConfig.maxFreq,
        sampleRate: _melConfig.sampleRate,
      );
      
      if (melResult != 0) {
        throw Exception('Failed to initialize mel processor: ${NativeBridgeWrapper.getLastError()}');
      }
      
      // Initialize OpenGL texture renderer if enabled
      if (_useOpenGL) {
        _logger.info('Initializing OpenGL texture renderer: width=512, height=256, numMelBands=${_melConfig.numFilters}');
        
        final textureResult = NativeBridgeWrapper.initTextureRenderer(512, 256, _melConfig.numFilters);
        
        if (textureResult != 0) {
          _logger.warning('Failed to initialize texture renderer: ${NativeBridgeWrapper.getLastError()}. Falling back to software rendering.');
          _useOpenGL = false;
        } else {
          _textureId = NativeBridgeWrapper.getTextureId();
          _textureInitialized = true;
          _logger.info('OpenGL texture renderer initialized successfully, texture ID: $_textureId');
        }
      }
      
      _state = AudioState.ready;
      _logger.info('Audio service initialized successfully');
      notifyListeners();
      return true;
      
    } catch (e) {
      _logger.severe('Failed to initialize audio service: $e');
      _state = AudioState.error;
      _error = e.toString();
      notifyListeners();
      return false;
    }
  }
  
  Future<bool> startRecording() async {
    if (_state != AudioState.ready) {
      _logger.warning('Cannot start recording in state: $_state');
      return false;
    }
    
    try {
      _logger.info('Starting audio recording');
      final result = NativeBridgeWrapper.startRecording();
      
      if (result != 0) {
        throw Exception('Failed to start recording: ${NativeBridgeWrapper.getLastError()}');
      }
      
      _state = AudioState.recording;
      _startProcessingLoop();
      
      _logger.info('Audio recording started');
      notifyListeners();
      return true;
      
    } catch (e) {
      _logger.severe('Failed to start recording: $e');
      _state = AudioState.error;
      _error = e.toString();
      notifyListeners();
      return false;
    }
  }
  
  Future<bool> stopRecording() async {
    if (_state != AudioState.recording) {
      _logger.warning('Cannot stop recording in state: $_state');
      return false;
    }
    
    try {
      _logger.info('Stopping audio recording');
      _stopProcessingLoop();
      
      final result = NativeBridgeWrapper.stopRecording();
      
      if (result != 0) {
        throw Exception('Failed to stop recording: ${NativeBridgeWrapper.getLastError()}');
      }
      
      _state = AudioState.ready;
      
      _logger.info('Audio recording stopped');
      notifyListeners();
      return true;
      
    } catch (e) {
      _logger.severe('Failed to stop recording: $e');
      _state = AudioState.error;
      _error = e.toString();
      notifyListeners();
      return false;
    }
  }
  
  void updateConfig({AudioConfig? audioConfig, MelConfig? melConfig}) {
    if (_state == AudioState.recording) {
      _logger.warning('Cannot update config while recording');
      return;
    }
    
    if (audioConfig != null) {
      _config = audioConfig;
    }
    
    if (melConfig != null) {
      _melConfig = melConfig;
    }
    
    _logger.info('Configuration updated');
    notifyListeners();
  }
  
  void _startProcessingLoop() {
    _processingTimer = Timer.periodic(const Duration(milliseconds: 23), (_) {
      _processAudioFrame();
    });
  }
  
  void _stopProcessingLoop() {
    _processingTimer?.cancel();
    _processingTimer = null;
  }
  
  void _processAudioFrame() {
    final frameStopwatch = Stopwatch()..start();
    
    try {
      // Get real audio frame from native bridge
      final audioFrame = _captureAudioFrame();
      if (audioFrame == null || audioFrame.isEmpty) {
        _stats.framesDropped++;
        return;
      }
      
      _currentAudioFrame = audioFrame;
      _frameCounter++;
      
      // Process audio frame through mel spectrogram
      final melData = _processMelSpectrogram(audioFrame);
      if (melData == null || melData.isEmpty) {
        _stats.framesDropped++;
        return;
      }
      
      // Update OpenGL texture if enabled
      if (_useOpenGL && _textureInitialized) {
        final textureStopwatch = Stopwatch()..start();
        final success = NativeBridgeWrapper.updateTextureColumn(melData, _frameCounter % 512);
        textureStopwatch.stop();
        
        if (success == 1) {
          _stats.textureUpdates++;
        } else {
          _logger.warning('Failed to update texture: ${NativeBridgeWrapper.getLastError()}');
        }
      }
      
      frameStopwatch.stop();
      
      // Update statistics
      _updateStats(frameStopwatch.elapsedMicroseconds);
      
      // Stream mel data
      _melStreamController.add(melData);
      _statsStreamController.add(_stats);
      
      notifyListeners();
      
    } catch (e) {
      _stats.framesDropped++;
      _logger.warning('Failed to process audio frame: $e');
    }
  }
  
  Float32List? _captureAudioFrame() {
    try {
      // Capture audio frame from native bridge
      final audioFrame = NativeBridgeWrapper.getAudioFrame();
      return audioFrame;
    } catch (e) {
      _logger.warning('Failed to capture audio frame: $e');
      return null;
    }
  }
  
  Float32List? _processMelSpectrogram(Float32List audioFrame) {
    try {
      // Process audio frame through mel spectrogram
      final success = NativeBridgeWrapper.processAudioFrame() == 1;
      if (!success) {
        _logger.warning('Failed to process audio frame: ${NativeBridgeWrapper.getLastError()}');
        return null;
      }
      
      // Get mel data
      final melData = NativeBridgeWrapper.getMelData();
      return melData;
    } catch (e) {
      _logger.warning('Failed to process mel spectrogram: $e');
      return null;
    }
  }
  
  void _updateStats(int frameTimeMicroseconds) {
    _stats.framesProcessed++;
    _stats.lastFrameTime = frameTimeMicroseconds / 1000.0; // Convert to milliseconds
    
    // Calculate moving average processing time
    if (_stats.averageProcessingTime == 0.0) {
      _stats.averageProcessingTime = frameTimeMicroseconds.toDouble();
    } else {
      _stats.averageProcessingTime = 
          (_stats.averageProcessingTime * 0.9) + (frameTimeMicroseconds * 0.1);
    }
    
    // Calculate FPS
    _stats.currentFps = 1000000.0 / _stats.averageProcessingTime;
    
    // Calculate audio level from current frame
    if (_currentAudioFrame != null) {
      _stats.audioLevel = _calculateAudioLevel(_currentAudioFrame!);
    }
  }
  
  // OpenGL texture renderer methods
  int? get textureId => _textureId;
  bool get useOpenGL => _useOpenGL;
  bool get textureInitialized => _textureInitialized;
  
  void setUseOpenGL(bool useOpenGL) {
    if (_useOpenGL == useOpenGL) return;
    
    if (_state == AudioState.recording) {
      _logger.warning('Cannot change OpenGL setting while recording');
      return;
    }
    
    _useOpenGL = useOpenGL;
    if (_useOpenGL && _state == AudioState.ready) {
      // Reinitialize with OpenGL
      _initializeTextureRenderer();
    }
    notifyListeners();
  }
  
  void _initializeTextureRenderer() {
    try {
      final result = NativeBridgeWrapper.initTextureRenderer(512, 256, _melConfig.numFilters);
      if (result == 0) {
        _textureId = NativeBridgeWrapper.getTextureId();
        _textureInitialized = true;
        _logger.info('Texture renderer reinitialized, texture ID: $_textureId');
      } else {
        _logger.warning('Failed to reinitialize texture renderer: ${NativeBridgeWrapper.getLastError()}');
        _textureInitialized = false;
      }
    } catch (e) {
      _logger.warning('Error reinitializing texture renderer: $e');
      _textureInitialized = false;
    }
  }
  
  double _calculateAudioLevel(Float32List audioFrame) {
    if (audioFrame.isEmpty) return -60.0;
    
    // Calculate RMS
    double sum = 0.0;
    for (final sample in audioFrame) {
      sum += sample * sample;
    }
    final rms = math.sqrt(sum / audioFrame.length);
    
    // Convert to dB
    if (rms < 0.000001) return -60.0;
    return 20 * math.log(rms) / math.ln10;
  }
  
  @override
  void dispose() {
    _stopProcessingLoop();
    
    if (_state == AudioState.recording) {
      NativeBridgeWrapper.stopRecording();
    }
    
    // NativeBridge.dispose(); // Not needed for mock implementation
    
    _melStreamController.close();
    _statsStreamController.close();
    
    _state = AudioState.disposed;
    
    super.dispose();
  }
}
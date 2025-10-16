import 'dart:async';
import 'dart:typed_data';
import 'dart:ffi' as ffi;
import 'dart:io';
import 'package:flutter/material.dart';
import 'dart:math' as math;
import 'package:ffi/ffi.dart';

class OpenGLAudioVisualizer extends StatefulWidget {
  final Stream<Float32List> melDataStream;
  final int width;
  final int height;
  final int numMelBands;
  
  const OpenGLAudioVisualizer({
    super.key,
    required this.melDataStream,
    this.width = 512,
    this.height = 256,
    this.numMelBands = 128,
  });

  @override
  State<OpenGLAudioVisualizer> createState() => _OpenGLAudioVisualizerState();
}

class _OpenGLAudioVisualizerState extends State<OpenGLAudioVisualizer> {
  late StreamSubscription<Float32List> _melSubscription;
  late ffi.DynamicLibrary _nativeLib;
  late TextureRenderer _textureRenderer;
  
  bool _isInitialized = false;
  String? _error;
  int? _textureId;
  
  @override
  void initState() {
    super.initState();
    _initializeNativeLibrary();
    _initializeTextureRenderer();
    _subscribeToMelData();
  }
  
  void _initializeNativeLibrary() {
    try {
      if (Platform.isAndroid) {
        _nativeLib = ffi.DynamicLibrary.open('libflutter_sp.so');
      } else if (Platform.isIOS) {
        _nativeLib = ffi.DynamicLibrary.process();
      } else if (Platform.isMacOS) {
        _nativeLib = ffi.DynamicLibrary.open('libflutter_sp.dylib');
      } else if (Platform.isWindows) {
        _nativeLib = ffi.DynamicLibrary.open('flutter_sp.dll');
      } else {
        _nativeLib = ffi.DynamicLibrary.process();
      }
      
      _textureRenderer = TextureRenderer(_nativeLib);
      
    } catch (e) {
      setState(() {
        _error = 'Failed to load native library: $e';
      });
    }
  }
  
  void _initializeTextureRenderer() {
    if (_error != null) return;
    
    try {
      final result = _textureRenderer.initialize(
        widget.width,
        widget.height,
        widget.numMelBands,
      );
      
      if (result != 0) {
        setState(() {
          _error = 'Failed to initialize texture renderer: ${_textureRenderer.getError()}';
        });
        return;
      }
      
      _textureId = _textureRenderer.getTextureId();
      
      setState(() {
        _isInitialized = true;
      });
      
    } catch (e) {
      setState(() {
        _error = 'Failed to initialize texture renderer: $e';
      });
    }
  }
  
  void _subscribeToMelData() {
    _melSubscription = widget.melDataStream.listen(
      (melData) {
        if (_isInitialized) {
          _updateTexture(melData);
        }
      },
      onError: (error) {
        setState(() {
          _error = 'Error receiving mel data: $error';
        });
      },
    );
  }
  
  void _updateTexture(Float32List melData) {
    try {
      final result = _textureRenderer.updateColumn(melData);
      if (result != 0) {
        print('Failed to update texture: ${_textureRenderer.getError()}');
      }
    } catch (e) {
      print('Error updating texture: $e');
    }
  }
  
  @override
  Widget build(BuildContext context) {
    if (_error != null) {
      return Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            const Icon(Icons.error_outline, color: Colors.red, size: 48),
            const SizedBox(height: 16),
            Text(
              'Error: $_error',
              style: const TextStyle(color: Colors.red),
              textAlign: TextAlign.center,
            ),
          ],
        ),
      );
    }
    
    if (!_isInitialized || _textureId == null) {
      return const Center(
        child: CircularProgressIndicator(),
      );
    }
    
    return Container(
      decoration: BoxDecoration(
        border: Border.all(color: Colors.grey),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Texture(textureId: _textureId!),
    );
  }
  
  @override
  void dispose() {
    _melSubscription.cancel();
    _textureRenderer.cleanup();
    super.dispose();
  }
}

class TextureRenderer {
  final ffi.DynamicLibrary _lib;
  
  late final ffi.Pointer<ffi.NativeFunction<ffi.Int Function(ffi.Int, ffi.Int, ffi.Int)>> _initTextureRenderer;
  late final ffi.Pointer<ffi.NativeFunction<ffi.Int Function(ffi.Pointer<ffi.Float>, ffi.Int)>> _updateTextureColumn;
  late final ffi.Pointer<ffi.NativeFunction<ffi.UnsignedInt Function()>> _getTextureId;
  late final ffi.Pointer<ffi.NativeFunction<ffi.Pointer<ffi.Char> Function()>> _getErrorMessage;
  late final ffi.Pointer<ffi.NativeFunction<ffi.Void Function()>> _cleanup;
  
  TextureRenderer(this._lib) {
    _initTextureRenderer = _lib.lookup<ffi.NativeFunction<ffi.Int Function(ffi.Int, ffi.Int, ffi.Int)>>('init_texture_renderer');
    _updateTextureColumn = _lib.lookup<ffi.NativeFunction<ffi.Int Function(ffi.Pointer<ffi.Float>, ffi.Int)>>('update_texture_column');
    _getTextureId = _lib.lookup<ffi.NativeFunction<ffi.UnsignedInt Function()>>('get_texture_id');
    _getErrorMessage = _lib.lookup<ffi.NativeFunction<ffi.Pointer<ffi.Char> Function()>>('get_error_message');
    _cleanup = _lib.lookup<ffi.NativeFunction<ffi.Void Function()>>('cleanup');
  }
  
  int initialize(int width, int height, int numMelBands) {
    return _initTextureRenderer.asFunction<int Function(int, int, int)>()(width, height, numMelBands);
  }
  
  int updateColumn(Float32List melData) {
    return using((arena) {
      final buffer = arena<ffi.Float>(melData.length);
      for (int i = 0; i < melData.length; i++) {
        buffer[i] = melData[i];
      }
      return _updateTextureColumn.asFunction<int Function(ffi.Pointer<ffi.Float>, int)>()(buffer, melData.length);
    });
  }
  
  int getTextureId() {
    return _getTextureId.asFunction<int Function()>()();
  }
  
  String getError() {
    final errorPtr = _getErrorMessage.asFunction<ffi.Pointer<ffi.Char> Function()>()();
    return errorPtr.cast<Utf8>().toDartString();
  }
  
  void cleanup() {
    _cleanup.asFunction<void Function()>()();
  }
}
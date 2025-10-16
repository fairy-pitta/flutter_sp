import 'dart:async';
import 'dart:typed_data';
import 'package:flutter/material.dart';

class WaterfallSpectrogram extends StatefulWidget {
  final Stream<Float32List> melDataStream;
  final int numMelFilters;
  final double minFrequency;
  final double maxFrequency;
  final Color lowFrequencyColor;
  final Color highFrequencyColor;
  final int maxHistory;
  
  const WaterfallSpectrogram({
    super.key,
    required this.melDataStream,
    this.numMelFilters = 128,
    this.minFrequency = 20.0,
    this.maxFrequency = 20000.0,
    this.lowFrequencyColor = Colors.blue,
    this.highFrequencyColor = Colors.red,
    this.maxHistory = 200,
  });

  @override
  State<WaterfallSpectrogram> createState() => _WaterfallSpectrogramState();
}

class _WaterfallSpectrogramState extends State<WaterfallSpectrogram> {
  late StreamSubscription<Float32List> _melSubscription;
  final List<Float32List> _melHistory = [];
  final List<Color> _colorMap = [];
  
  String? _error;
  
  @override
  void initState() {
    super.initState();
    _initializeColorMap();
    _subscribeToMelData();
  }
  
  void _initializeColorMap() {
    _colorMap.clear();
    for (int i = 0; i < 256; i++) {
      final t = i / 255.0;
      _colorMap.add(Color.lerp(
        widget.lowFrequencyColor,
        widget.highFrequencyColor,
        t,
      )!);
    }
  }
  
  void _subscribeToMelData() {
    _melSubscription = widget.melDataStream.listen(
      (melData) {
        _updateMelHistory(melData);
      },
      onError: (error) {
        setState(() {
          _error = 'Error receiving mel data: $error';
        });
      },
    );
  }
  
  void _updateMelHistory(Float32List melData) {
    setState(() {
      _melHistory.add(Float32List.fromList(melData));
      
      if (_melHistory.length > widget.maxHistory) {
        _melHistory.removeAt(0);
      }
    });
  }
  
  @override
  Widget build(BuildContext context) {
    if (_error != null) {
      return Center(
        child: Text(
          'Error: $_error',
          style: const TextStyle(color: Colors.red),
        ),
      );
    }
    
    return Container(
      decoration: BoxDecoration(
        border: Border.all(color: Colors.grey),
        borderRadius: BorderRadius.circular(8),
      ),
      child: CustomPaint(
        painter: SpectrogramPainter(
          melHistory: _melHistory,
          numMelFilters: widget.numMelFilters,
          colorMap: _colorMap,
        ),
        size: Size.infinite,
      ),
    );
  }
  
  @override
  void dispose() {
    _melSubscription.cancel();
    super.dispose();
  }
}

class SpectrogramPainter extends CustomPainter {
  final List<Float32List> melHistory;
  final int numMelFilters;
  final List<Color> colorMap;
  
  SpectrogramPainter({
    required this.melHistory,
    required this.numMelFilters,
    required this.colorMap,
  });
  
  @override
  void paint(Canvas canvas, Size size) {
    if (melHistory.isEmpty) return;
    
    final cellWidth = size.width / numMelFilters;
    final cellHeight = size.height / melHistory.length;
    
    for (int y = 0; y < melHistory.length; y++) {
      final melData = melHistory[y];
      for (int x = 0; x < numMelFilters; x++) {
        final melValue = melData[x.clamp(0, melData.length - 1)];
        final intensity = melValue.clamp(0.0, 1.0);
        final color = colorMap[(intensity * 255).clamp(0, 255).round()];
        
        final rect = Rect.fromLTWH(
          x * cellWidth,
          y * cellHeight,
          cellWidth,
          cellHeight,
        );
        
        final paint = Paint()..color = color;
        canvas.drawRect(rect, paint);
      }
    }
  }
  
  @override
  bool shouldRepaint(SpectrogramPainter oldDelegate) {
    return melHistory != oldDelegate.melHistory;
  }
}
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'audio_service.dart';

class SettingsPanel extends StatefulWidget {
  final VoidCallback? onClose;
  
  const SettingsPanel({Key? key, this.onClose}) : super(key: key);
  
  @override
  _SettingsPanelState createState() => _SettingsPanelState();
}

class _SettingsPanelState extends State<SettingsPanel> {
  bool _showAdvanced = false;
  
  @override
  Widget build(BuildContext context) {
    return Consumer<AudioService>(
      builder: (context, audioService, child) {
        return Container(
          width: 400,
          padding: const EdgeInsets.all(16),
          decoration: BoxDecoration(
            color: Colors.grey[900],
            borderRadius: BorderRadius.circular(12),
            border: Border.all(color: Colors.grey[700]!),
          ),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              // Header
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text(
                    'Settings',
                    style: TextStyle(
                      color: Colors.white,
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  IconButton(
                    icon: Icon(Icons.close, color: Colors.white),
                    onPressed: widget.onClose,
                  ),
                ],
              ),
              const SizedBox(height: 16),
              
              // Rendering Options
              _buildSectionTitle('Rendering'),
              _buildOpenGLToggle(audioService),
              const SizedBox(height: 16),
              
              // Audio Configuration
              _buildSectionTitle('Audio Configuration'),
              _buildAudioConfigSection(audioService),
              const SizedBox(height: 16),
              
              // Performance Monitoring
              _buildSectionTitle('Performance'),
              _buildPerformanceSection(audioService),
              const SizedBox(height: 16),
              
              // Advanced Options
              _buildAdvancedSection(audioService),
              
              // Action Buttons
              const SizedBox(height: 24),
              Row(
                mainAxisAlignment: MainAxisAlignment.end,
                children: [
                  TextButton(
                    onPressed: widget.onClose,
                    child: Text('Close', style: TextStyle(color: Colors.white)),
                  ),
                ],
              ),
            ],
          ),
        );
      },
    );
  }
  
  Widget _buildSectionTitle(String title) {
    return Text(
      title,
      style: TextStyle(
        color: Colors.grey[300],
        fontSize: 14,
        fontWeight: FontWeight.w500,
      ),
    );
  }
  
  Widget _buildOpenGLToggle(AudioService audioService) {
    return Container(
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        color: Colors.grey[800],
        borderRadius: BorderRadius.circular(8),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text(
                'GPU Acceleration (OpenGL)',
                style: TextStyle(color: Colors.white),
              ),
              Switch(
                value: audioService.useOpenGL,
                onChanged: (value) {
                  audioService.setUseOpenGL(value);
                },
                activeColor: Colors.blue,
              ),
            ],
          ),
          const SizedBox(height: 8),
          Text(
            audioService.useOpenGL 
                ? 'Using OpenGL texture rendering for optimal performance'
                : 'Using software rendering (fallback mode)',
            style: TextStyle(
              color: Colors.grey[400],
              fontSize: 12,
            ),
          ),
          if (audioService.textureInitialized) ...[
            const SizedBox(height: 8),
            Text(
              'Texture ID: ${audioService.textureId ?? "N/A"}',
              style: TextStyle(
                color: Colors.green[300],
                fontSize: 12,
              ),
            ),
          ],
        ],
      ),
    );
  }
  
  Widget _buildAudioConfigSection(AudioService audioService) {
    return Container(
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        color: Colors.grey[800],
        borderRadius: BorderRadius.circular(8),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          _buildConfigRow('Sample Rate', '${audioService.config.sampleRate} Hz'),
          _buildConfigRow('Buffer Size', '${audioService.config.bufferSize} samples'),
          _buildConfigRow('Channels', '${audioService.config.channels}'),
          _buildConfigRow('Format', audioService.config.format.toString().split('.').last),
          const SizedBox(height: 8),
          _buildConfigRow('Mel Filters', '${audioService.melConfig.numFilters}'),
          _buildConfigRow('Frequency Range', '${audioService.melConfig.minFreq.toInt()}-${audioService.melConfig.maxFreq.toInt()} Hz'),
        ],
      ),
    );
  }
  
  Widget _buildConfigRow(String label, String value) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 2),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label, style: TextStyle(color: Colors.grey[400], fontSize: 12)),
          Text(value, style: TextStyle(color: Colors.white, fontSize: 12)),
        ],
      ),
    );
  }
  
  Widget _buildPerformanceSection(AudioService audioService) {
    return StreamBuilder<AudioProcessingStats>(
      stream: audioService.statsStream,
      initialData: audioService.processingStats,
      builder: (context, snapshot) {
        final stats = snapshot.data;
        if (stats == null) return SizedBox.shrink();
        
        return Container(
          padding: const EdgeInsets.all(12),
          decoration: BoxDecoration(
            color: Colors.grey[800],
            borderRadius: BorderRadius.circular(8),
          ),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              _buildPerformanceRow('FPS', '${stats.currentFps.toStringAsFixed(1)}'),
              _buildPerformanceRow('Frame Time', '${stats.lastFrameTime.toStringAsFixed(1)} ms'),
              _buildPerformanceRow('Avg Processing', '${(stats.averageProcessingTime / 1000).toStringAsFixed(2)} ms'),
              _buildPerformanceRow('Frames Processed', '${stats.framesProcessed}'),
              _buildPerformanceRow('Frames Dropped', '${stats.framesDropped}'),
              _buildPerformanceRow('Texture Updates', '${stats.textureUpdates}'),
              const SizedBox(height: 8),
              _buildPerformanceRow('Audio Level', '${stats.audioLevel.toStringAsFixed(1)} dB'),
            ],
          ),
        );
      },
    );
  }
  
  Widget _buildPerformanceRow(String label, String value) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 1),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label, style: TextStyle(color: Colors.grey[400], fontSize: 11)),
          Text(value, style: TextStyle(color: Colors.white, fontSize: 11)),
        ],
      ),
    );
  }
  
  Widget _buildAdvancedSection(AudioService audioService) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        TextButton(
          onPressed: () {
            setState(() {
              _showAdvanced = !_showAdvanced;
            });
          },
          child: Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              Text('Advanced', style: TextStyle(color: Colors.blue)),
              Icon(
                _showAdvanced ? Icons.expand_less : Icons.expand_more,
                color: Colors.blue,
                size: 16,
              ),
            ],
          ),
        ),
        if (_showAdvanced) ...[
          const SizedBox(height: 8),
          Container(
            padding: const EdgeInsets.all(12),
            decoration: BoxDecoration(
              color: Colors.grey[800],
              borderRadius: BorderRadius.circular(8),
            ),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  'Color Maps',
                  style: TextStyle(color: Colors.white, fontSize: 12),
                ),
                const SizedBox(height: 8),
                Row(
                  children: [
                    _buildColorMapButton('Viridis', 0),
                    const SizedBox(width: 8),
                    _buildColorMapButton('Plasma', 1),
                    const SizedBox(width: 8),
                    _buildColorMapButton('Inferno', 2),
                  ],
                ),
                const SizedBox(height: 12),
                Text(
                  'Texture Range',
                  style: TextStyle(color: Colors.white, fontSize: 12),
                ),
                const SizedBox(height: 8),
                Row(
                  children: [
                    Expanded(
                      child: TextField(
                        decoration: InputDecoration(
                          labelText: 'Min',
                          labelStyle: TextStyle(color: Colors.grey[400], fontSize: 10),
                          border: OutlineInputBorder(),
                          contentPadding: EdgeInsets.symmetric(horizontal: 8, vertical: 4),
                        ),
                        style: TextStyle(color: Colors.white, fontSize: 10),
                        keyboardType: TextInputType.number,
                        onSubmitted: (value) {
                          // TODO: Implement min range setting
                        },
                      ),
                    ),
                    const SizedBox(width: 8),
                    Expanded(
                      child: TextField(
                        decoration: InputDecoration(
                          labelText: 'Max',
                          labelStyle: TextStyle(color: Colors.grey[400], fontSize: 10),
                          border: OutlineInputBorder(),
                          contentPadding: EdgeInsets.symmetric(horizontal: 8, vertical: 4),
                        ),
                        style: TextStyle(color: Colors.white, fontSize: 10),
                        keyboardType: TextInputType.number,
                        onSubmitted: (value) {
                          // TODO: Implement max range setting
                        },
                      ),
                    ),
                  ],
                ),
              ],
            ),
          ),
        ],
      ],
    );
  }
  
  Widget _buildColorMapButton(String name, int type) {
    return TextButton(
      onPressed: () {
        // TODO: Implement color map setting
      },
      child: Text(
        name,
        style: TextStyle(color: Colors.blue, fontSize: 10),
      ),
      style: TextButton.styleFrom(
        padding: EdgeInsets.symmetric(horizontal: 8, vertical: 4),
        backgroundColor: Colors.grey[700],
      ),
    );
  }
}
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'audio_service.dart';
import 'audio_visualizer.dart';

void main() {
  runApp(const MelSpectrogramApp());
}

class MelSpectrogramApp extends StatelessWidget {
  const MelSpectrogramApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Mel Spectrogram',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        brightness: Brightness.dark,
        useMaterial3: true,
      ),
      home: ChangeNotifierProvider(
        create: (context) => AudioService(),
        child: const MainScreen(),
      ),
    );
  }
}

class MainScreen extends StatefulWidget {
  const MainScreen({super.key});

  @override
  State<MainScreen> createState() => _MainScreenState();
}

class _MainScreenState extends State<MainScreen> {
  AudioService? _audioService;
  
  @override
  void initState() {
    super.initState();
    // Defer initialization to after the widget is built
    WidgetsBinding.instance.addPostFrameCallback((_) {
      _initializeAudio();
    });
  }
  
  Future<void> _initializeAudio() async {
    try {
      final audioService = context.read<AudioService>();
      _audioService = audioService;
      await audioService.initialize();
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Failed to initialize audio: $e')),
        );
      }
    }
  }
  
  void _toggleRecording() async {
    final audioService = _audioService;
    if (audioService == null) return;
    
    try {
      if (audioService.isRecording) {
        await audioService.stopRecording();
      } else {
        await audioService.startRecording();
      }
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Recording error: $e')),
        );
      }
    }
  }
  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Mel Spectrogram'),
        actions: [
          IconButton(
            icon: const Icon(Icons.settings),
            onPressed: () => _showSettingsDialog(),
          ),
        ],
      ),
      body: Column(
        children: [
          // Status panel
          Consumer<AudioService>(
            builder: (context, audioService, child) {
              return Container(
                padding: const EdgeInsets.all(16),
                color: Colors.grey[900],
                child: Row(
                  children: [
                    Icon(
                      audioService.isRecording ? Icons.circle : Icons.circle_outlined,
                      color: audioService.isRecording ? Colors.red : Colors.grey,
                      size: 24,
                    ),
                    const SizedBox(width: 8),
                    Text(
                      audioService.isRecording ? 'Recording' : 'Idle',
                      style: const TextStyle(fontSize: 16),
                    ),
                    const Spacer(),
                    if (audioService.audioLevel != null) ...[
                      Text('Level: ${audioService.audioLevel!.toStringAsFixed(1)} dB'),
                      const SizedBox(width: 16),
                    ],
                    if (audioService.processingStats != null) ...[
                      Text('FPS: ${audioService.processingStats!.currentFps.toStringAsFixed(1)}'),
                    ],
                  ],
                ),
              );
            },
          ),
          
          // Spectrogram display
          Expanded(
            child: Consumer<AudioService>(
              builder: (context, audioService, child) {
                if (!audioService.isInitialized) {
                  return const Center(
                    child: CircularProgressIndicator(),
                  );
                }
                
                return Padding(
                  padding: const EdgeInsets.all(16),
                  child: WaterfallSpectrogram(
                    melDataStream: audioService.melDataStream,
                    numMelFilters: audioService.melConfig.numFilters,
                    minFrequency: audioService.melConfig.minFreq,
                    maxFrequency: audioService.melConfig.maxFreq,
                  ),
                );
              },
            ),
          ),
          
          // Control panel
          Container(
            padding: const EdgeInsets.all(16),
            color: Colors.grey[900],
            child: Consumer<AudioService>(
              builder: (context, audioService, child) {
                return Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    ElevatedButton.icon(
                      onPressed: _toggleRecording,
                      icon: Icon(
                        audioService.isRecording ? Icons.stop : Icons.mic,
                      ),
                      label: Text(
                        audioService.isRecording ? 'Stop' : 'Start',
                      ),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: audioService.isRecording ? Colors.red : Colors.green,
                        padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 12),
                      ),
                    ),
                    const SizedBox(width: 16),
                    if (audioService.error != null)
                      Text(
                        'Error: ${audioService.error}',
                        style: const TextStyle(color: Colors.red),
                      ),
                  ],
                );
              },
            ),
          ),
        ],
      ),
    );
  }
  
  void _showSettingsDialog() {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Settings'),
        content: Consumer<AudioService>(
          builder: (context, audioService, child) {
            return Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                ListTile(
                  title: const Text('Sample Rate'),
                  subtitle: Text('${audioService.config.sampleRate} Hz'),
                ),
                ListTile(
                  title: const Text('Buffer Size'),
                  subtitle: Text('${audioService.config.bufferSize} samples'),
                ),
                ListTile(
                  title: const Text('Mel Filters'),
                  subtitle: Text('${audioService.melConfig.numFilters}'),
                ),
                ListTile(
                  title: const Text('Min Frequency'),
                  subtitle: Text('${audioService.melConfig.minFreq} Hz'),
                ),
                ListTile(
                  title: const Text('Max Frequency'),
                  subtitle: Text('${audioService.melConfig.maxFreq} Hz'),
                ),
              ],
            );
          },
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('Close'),
          ),
        ],
      ),
    );
  }
}
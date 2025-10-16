import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../services/audio_service.dart';
import '../services/spectrogram_service.dart';
import '../widgets/spectrogram_widget.dart';
import '../widgets/control_panel.dart';
import '../widgets/status_bar.dart';

class MainScreen extends StatefulWidget {
  @override
  _MainScreenState createState() => _MainScreenState();
}

class _MainScreenState extends State<MainScreen> {
  @override
  void initState() {
    super.initState();
    _initializeServices();
  }

  Future<void> _initializeServices() async {
    final audioService = Provider.of<AudioService>(context, listen: false);
    final spectrogramService = Provider.of<SpectrogramService>(context, listen: false);
    
    await audioService.initialize();
    spectrogramService.initialize();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Mel Spectrogram'),
        backgroundColor: Color(0xFF1a1a1a),
        actions: [
          IconButton(
            icon: Icon(Icons.settings),
            onPressed: () => Navigator.pushNamed(context, '/settings'),
          ),
          IconButton(
            icon: Icon(Icons.save_alt),
            onPressed: () => Navigator.pushNamed(context, '/export'),
          ),
        ],
      ),
      body: Column(
        children: [
          Expanded(
            flex: 3,
            child: Consumer<SpectrogramService>(
              builder: (context, service, child) {
                return SpectrogramWidget(
                  spectrogramData: service.currentData,
                  isActive: service.isProcessing,
                );
              },
            ),
          ),
          StatusBar(),
          ControlPanel(),
        ],
      ),
    );
  }

  @override
  void dispose() {
    Provider.of<AudioService>(context, listen: false).dispose();
    Provider.of<SpectrogramService>(context, listen: false).dispose();
    super.dispose();
  }
}
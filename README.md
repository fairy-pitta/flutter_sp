# Real-time Mel Spectrogram Waterfall

A high-performance mobile application for real-time mel spectrogram visualization with GPU-accelerated rendering and cross-platform support.

## 🎯 Features

- **Real-time Processing**: ≤ 200ms latency with 16-25 FPS
- **Cross-platform**: iOS and Android support
- **GPU Acceleration**: OpenGL ES 3.0 (Android) / Metal (iOS)
- **Professional DSP**: C++ backend with KissFFT
- **Intuitive UI**: Flutter-based interface
- **Performance Optimized**: Thermal management and adaptive quality
- **Export Functionality**: Save spectrograms as images/data

## 🏗️ Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Flutter UI    │───▶│  Platform       │───▶│   Native        │
│                 │    │  Channels       │    │   Services      │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                │                       │
                                ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Spectrogram   │◀───│   DSP Engine    │◀───│   Audio I/O     │
│   Display       │    │   (C++)         │    │   (Platform)    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                │                       │
                                ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   GPU Texture   │◀───│   Rendering     │◀───│   Ring Buffer   │
│   Update        │    │   (OpenGL/Metal)│    │   (Lock-free)   │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## 🚀 Performance Specifications

| Metric | Target | Current |
|--------|--------|---------|
| Latency | ≤ 200ms | TBD |
| Frame Rate | 16-25 FPS | TBD |
| CPU Usage | ≤ 15% | TBD |
| Memory | ≤ 50MB | TBD |
| Thermal | No throttling | TBD |

## 📱 Technical Specifications

### Audio Processing
- **Sample Rate**: 32kHz (configurable: 16k/32k/44.1kHz)
- **Frame Size**: 1024 samples
- **Hop Size**: 512 samples (~16ms)
- **Mel Bands**: 64 (configurable)
- **Format**: PCM16 mono

### Display
- **Resolution**: 512×256 pixels (expandable to 1024×256)
- **Color Maps**: Viridis, Inferno, Plasma
- **Update Rate**: 40-60ms intervals
- **Texture Format**: RGBA

## 🛠️ Development Setup

### Prerequisites
- Flutter 3.16+
- Dart 3.0+
- CMake 3.14+
- C++17 compatible compiler
- Android Studio / Xcode

### Installation

1. **Clone the repository**
```bash
git clone https://github.com/your-repo/mel-spectrogram.git
cd mel-spectrogram
```

2. **Install Flutter dependencies**
```bash
flutter pub get
```

3. **Build C++ DSP library**
```bash
cd native
mkdir build && cd build
cmake ..
make -j$(nproc)
```

4. **Run tests**
```bash
# C++ tests
make test

# Flutter tests
flutter test
```

5. **Run the app**
```bash
# Android
flutter run -d android

# iOS
flutter run -d ios
```

## 🧪 Testing Strategy

### Unit Tests
- DSP algorithm accuracy
- UI component functionality
- Platform channel integration
- Performance benchmarks

### Integration Tests
- End-to-end processing pipeline
- Cross-platform compatibility
- Performance under load
- Thermal stability

### Test-Driven Development (TDD)
Following TDD principles by t-wada:
1. Write tests first
2. Make tests pass
3. Refactor code
4. Never force tests to pass by changing expected values

```cpp
// Example TDD cycle
TEST(MelSpectrogramTest, FrequencyDetection) {
    // Given
    MelSpectrogramProcessor processor(config);
    std::vector<int16_t> signal = generateSineWave(1000.0f);
    
    // When
    processor.processAudioFrame(signal.data(), signal.size());
    auto spectrum = processor.getMelSpectrum();
    
    // Then
    EXPECT_NEAR(findPeakFrequency(spectrum), 1000.0f, 50.0f);
}
```

## 📊 Development Milestones

| Milestone | Duration | Status |
|-----------|----------|--------|
| Project Setup | 1 week | ✅ Complete |
| Audio Input | 1 week | 🚧 In Progress |
| DSP Engine | 1 week | 📋 Planned |
| GPU Rendering | 1 week | 📋 Planned |
| Flutter Integration | 1 week | 📋 Planned |
| Performance Optimization | 1 week | 📋 Planned |
| Testing & QA | 1 week | 📋 Planned |
| Deployment | 1 week | 📋 Planned |

## 🔧 Configuration

### Audio Settings
```json
{
  "sampleRate": 32000,
  "frameSize": 1024,
  "hopSize": 512,
  "numMelBands": 64,
  "minFreq": 20.0,
  "maxFreq": 8000.0
}
```

### Display Settings
```json
{
  "colorScheme": "viridis",
  "updateInterval": 50,
  "timeWindow": 10.0,
  "frequencyRange": [20, 8000]
}
```

### Performance Settings
```json
{
  "quality": "balanced",
  "powerSaving": true,
  "adaptiveQuality": true,
  "thermalThrottling": true
}
```

## 📈 Performance Monitoring

Built-in performance monitoring includes:
- Processing latency measurement
- Frame rate tracking
- CPU usage monitoring
- Thermal state detection
- Memory usage tracking

```cpp
struct ProcessingStats {
    float processingTimeMs;
    float fps;
    float cpuUsage;
    int droppedFrames;
    float temperature;
};
```

## 🎨 Customization

### Color Maps
- Viridis (default)
- Inferno
- Plasma
- Custom RGB gradients

### Export Formats
- PNG/JPEG images
- CSV data
- JSON metadata
- MP4 video (Premium)

## 🔒 Permissions

Required permissions:
- **Microphone**: Audio input for spectrogram
- **Storage**: Export functionality
- **Camera** (optional): Enhanced export features

Privacy-first approach:
- No audio data leaves the device
- Processing happens locally
- No cloud services required
- Minimal data collection

## 🐛 Troubleshooting

### Common Issues

**High Latency**
- Check CPU usage and thermal state
- Reduce quality settings
- Close background apps

**Audio Permission Denied**
- Check system settings
- Restart the app
- Reinstall if necessary

**Crashes on Startup**
- Update Flutter and dependencies
- Clear build cache
- Check device compatibility

**Poor Performance**
- Enable power saving mode
- Reduce display resolution
- Disable adaptive quality

## 🤝 Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Write tests first (TDD)
4. Implement functionality
5. Run full test suite
6. Commit changes (`git commit -m 'Add amazing feature'`)
7. Push to branch (`git push origin feature/amazing-feature`)
8. Open Pull Request

### Code Style
- Follow Flutter/Dart style guide
- Use C++17 best practices
- Write comprehensive tests
- Document public APIs
- Follow TDD principles

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- KissFFT for fast Fourier transform
- Flutter team for cross-platform framework
- Audio engineering community for algorithms
- TDD practitioners for development methodology

## 📞 Support

- 📧 Email: support@mel-spectrogram.com
- 💬 Discord: [Join our server](https://discord.gg/mel-spectrogram)
- 📚 Documentation: [Wiki](https://github.com/your-repo/mel-spectrogram/wiki)
- 🐛 Issues: [GitHub Issues](https://github.com/your-repo/mel-spectrogram/issues)

---

**Built with ❤️ using Flutter, C++, and TDD principles**
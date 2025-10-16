#ifndef MEL_SPECTROGRAM_H
#define MEL_SPECTROGRAM_H

#include <vector>
#include <complex>
#include <memory>

namespace melspectrogram {

struct AudioConfig {
    int sampleRate = 32000;
    int frameSize = 1024;
    int hopSize = 512;
    int numMelBands = 64;
    float minFreq = 20.0f;
    float maxFreq = 8000.0f;
};

struct ProcessingStats {
    float processingTimeMs = 0.0f;
    float fps = 0.0f;
    float cpuUsage = 0.0f;
    int droppedFrames = 0;
};

class MelSpectrogramProcessor {
public:
    explicit MelSpectrogramProcessor(const AudioConfig& config);
    ~MelSpectrogramProcessor();

    // Main processing function
    bool processAudioFrame(const int16_t* input, size_t inputSize);
    
    // Get processing results
    std::vector<float> getMelSpectrum() const;
    std::vector<uint8_t> getColorMappedData() const;
    ProcessingStats getStats() const { return stats_; }
    
    // Configuration updates
    void updateConfig(const AudioConfig& config);
    void setColorMap(const std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>& colormap);
    
    // Performance monitoring
    void resetStats();
    bool isOverloaded() const;

private:
    // Internal processing steps
    void applyWindowFunction();
    void performFFT();
    void computePowerSpectrum();
    void applyMelFilterBank();
    void convertToLogScale();
    void applyColorMapping();
    
    // Helper functions
    void createMelFilterBank();
    float freqToMel(float freq) const;
    float melToFreq(float mel) const;
    void createWindowFunction();
    
    // Member variables
    AudioConfig config_;
    ProcessingStats stats_;
    
    // Processing buffers
    std::vector<float> windowFunction_;
    std::vector<std::complex<float>> fftInput_;
    std::vector<std::complex<float>> fftOutput_;
    std::vector<float> powerSpectrum_;
    std::vector<float> melSpectrum_;
    std::vector<uint8_t> colorMappedData_;
    
    // Mel filter bank
    std::vector<std::vector<float>> melFilterBank_;
    
    // Color mapping
    std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> colorMap_;
    
    // FFT implementation (KissFFT)
    void* kissFFTConfig_;
    
    // Performance tracking
    std::chrono::high_resolution_clock::time_point lastFrameTime_;
    int frameCount_ = 0;
};

// Utility functions
std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> createViridisColorMap();
std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> createInfernoColorMap();
std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> createPlasmaColorMap();

} // namespace melspectrogram

#endif // MEL_SPECTROGRAM_H
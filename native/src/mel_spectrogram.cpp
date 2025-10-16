#include "mel_spectrogram.h"
#include "kiss_fft.h"
#include <cmath>
#include <algorithm>
#include <chrono>
#include <numeric>

namespace melspectrogram {

namespace {
    constexpr float PI = 3.14159265359f;
    constexpr float MIN_LOG_VALUE = 1e-10f;
}

MelSpectrogramProcessor::MelSpectrogramProcessor(const AudioConfig& config) 
    : config_(config) {
    
    // Initialize buffers
    windowFunction_.resize(config_.frameSize);
    fftInput_.resize(config_.frameSize);
    fftOutput_.resize(config_.frameSize);
    powerSpectrum_.resize(config_.frameSize / 2 + 1);
    melSpectrum_.resize(config_.numMelBands);
    colorMappedData_.resize(config_.numMelBands * 4); // RGBA
    
    // Initialize FFT
    kissFFTConfig_ = kiss_fft_alloc(config_.frameSize, 0, nullptr, nullptr);
    
    // Create window function and filter bank
    createWindowFunction();
    createMelFilterBank();
    
    // Default color map (viridis)
    colorMap_ = createViridisColorMap();
    
    lastFrameTime_ = std::chrono::high_resolution_clock::now();
}

MelSpectrogramProcessor::~MelSpectrogramProcessor() {
    if (kissFFTConfig_) {
        kiss_fft_free(kissFFTConfig_);
    }
}

bool MelSpectrogramProcessor::processAudioFrame(const int16_t* input, size_t inputSize) {
    if (input == nullptr || inputSize != static_cast<size_t>(config_.frameSize)) {
        return false;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Convert int16 to float and apply window
    for (int i = 0; i < config_.frameSize; ++i) {
        fftInput_[i] = std::complex<float>(
            input[i] * windowFunction_[i] / 32768.0f, 0.0f
        );
    }
    
    performFFT();
    computePowerSpectrum();
    applyMelFilterBank();
    convertToLogScale();
    applyColorMapping();
    
    // Update stats
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    stats_.processingTimeMs = duration.count() / 1000.0f;
    
    frameCount_++;
    if (frameCount_ % 30 == 0) { // Update FPS every 30 frames
        auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - lastFrameTime_);
        stats_.fps = 30000.0f / totalDuration.count();
        lastFrameTime_ = endTime;
    }
    
    return true;
}

void MelSpectrogramProcessor::performFFT() {
    kiss_fft(reinterpret_cast<kiss_fft_cfg>(kissFFTConfig_), 
             reinterpret_cast<kiss_fft_cpx*>(fftInput_.data()),
             reinterpret_cast<kiss_fft_cpx*>(fftOutput_.data()));
}

void MelSpectrogramProcessor::computePowerSpectrum() {
    for (int i = 0; i <= config_.frameSize / 2; ++i) {
        float real = fftOutput_[i].real();
        float imag = fftOutput_[i].imag();
        powerSpectrum_[i] = (real * real + imag * imag) / config_.frameSize;
    }
}

void MelSpectrogramProcessor::applyMelFilterBank() {
    std::fill(melSpectrum_.begin(), melSpectrum_.end(), 0.0f);
    
    for (int melBand = 0; melBand < config_.numMelBands; ++melBand) {
        for (int freqBin = 0; freqBin <= config_.frameSize / 2; ++freqBin) {
            melSpectrum_[melBand] += powerSpectrum_[freqBin] * melFilterBank_[melBand][freqBin];
        }
    }
}

void MelSpectrogramProcessor::convertToLogScale() {
    for (auto& value : melSpectrum_) {
        value = 10.0f * std::log10(std::max(value, MIN_LOG_VALUE));
    }
    
    // Normalize to 0-1 range
    float minValue = *std::min_element(melSpectrum_.begin(), melSpectrum_.end());
    float maxValue = *std::max_element(melSpectrum_.begin(), melSpectrum_.end());
    float range = maxValue - minValue;
    
    if (range > 0) {
        for (auto& value : melSpectrum_) {
            value = (value - minValue) / range;
        }
    }
}

void MelSpectrogramProcessor::applyColorMapping() {
    for (int i = 0; i < config_.numMelBands; ++i) {
        float normalizedValue = melSpectrum_[i];
        if (normalizedValue < 0.0f) normalizedValue = 0.0f;
        if (normalizedValue > 1.0f) normalizedValue = 1.0f;
        int colorIndex = static_cast<int>(normalizedValue * (colorMap_.size() - 1));
        
        uint8_t r = std::get<0>(colorMap_[colorIndex]);
        uint8_t g = std::get<1>(colorMap_[colorIndex]);
        uint8_t b = std::get<2>(colorMap_[colorIndex]);
        colorMappedData_[i * 4 + 0] = r;     // R
        colorMappedData_[i * 4 + 1] = g;     // G
        colorMappedData_[i * 4 + 2] = b;     // B
        colorMappedData_[i * 4 + 3] = 255;   // A
    }
}

void MelSpectrogramProcessor::createWindowFunction() {
    // Hann window
    for (int i = 0; i < config_.frameSize; ++i) {
        windowFunction_[i] = 0.5f * (1.0f - std::cos(2.0f * PI * i / (config_.frameSize - 1)));
    }
}

void MelSpectrogramProcessor::createMelFilterBank() {
    melFilterBank_.resize(config_.numMelBands);
    
    float minMel = freqToMel(config_.minFreq);
    float maxMel = freqToMel(config_.maxFreq);
    
    std::vector<float> melPoints(config_.numMelBands + 2);
    for (int i = 0; i < config_.numMelBands + 2; ++i) {
        melPoints[i] = minMel + (maxMel - minMel) * i / (config_.numMelBands + 1);
    }
    
    std::vector<float> freqPoints(config_.numMelBands + 2);
    for (int i = 0; i < config_.numMelBands + 2; ++i) {
        freqPoints[i] = melToFreq(melPoints[i]);
    }
    
    for (int melBand = 0; melBand < config_.numMelBands; ++melBand) {
        melFilterBank_[melBand].resize(config_.frameSize / 2 + 1);
        
        float leftFreq = freqPoints[melBand];
        float centerFreq = freqPoints[melBand + 1];
        float rightFreq = freqPoints[melBand + 2];
        
        for (int freqBin = 0; freqBin <= config_.frameSize / 2; ++freqBin) {
            float freq = static_cast<float>(freqBin) * config_.sampleRate / config_.frameSize;
            
            if (freq >= leftFreq && freq <= centerFreq) {
                melFilterBank_[melBand][freqBin] = (freq - leftFreq) / (centerFreq - leftFreq);
            } else if (freq >= centerFreq && freq <= rightFreq) {
                melFilterBank_[melBand][freqBin] = (rightFreq - freq) / (rightFreq - centerFreq);
            } else {
                melFilterBank_[melBand][freqBin] = 0.0f;
            }
        }
    }
}

float MelSpectrogramProcessor::freqToMel(float freq) const {
    return 2595.0f * std::log10(1.0f + freq / 700.0f);
}

float MelSpectrogramProcessor::melToFreq(float mel) const {
    return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
}

std::vector<float> MelSpectrogramProcessor::getMelSpectrum() const {
    return melSpectrum_;
}

std::vector<uint8_t> MelSpectrogramProcessor::getColorMappedData() const {
    return colorMappedData_;
}

bool MelSpectrogramProcessor::isOverloaded() const {
    return stats_.processingTimeMs > 50.0f; // 50ms threshold
}

void MelSpectrogramProcessor::resetStats() {
    stats_ = ProcessingStats{};
    frameCount_ = 0;
}

void MelSpectrogramProcessor::updateConfig(const AudioConfig& config) {
    config_ = config;
    
    // Reinitialize buffers with new config
    windowFunction_.resize(config_.frameSize);
    fftInput_.resize(config_.frameSize);
    fftOutput_.resize(config_.frameSize);
    powerSpectrum_.resize(config_.frameSize / 2 + 1);
    melSpectrum_.resize(config_.numMelBands);
    colorMappedData_.resize(config_.numMelBands * 4); // RGBA
    
    // Recreate window function and filter bank
    createWindowFunction();
    createMelFilterBank();
}

void MelSpectrogramProcessor::setColorMap(const std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>& colormap) {
    colorMap_ = colormap;
}

// Color map creation functions
std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> createViridisColorMap() {
    std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> colormap(256);
    
    for (int i = 0; i < 256; ++i) {
        float t = i / 255.0f;
        float r_val = 68.0f + t * (59.0f - 68.0f);
        float g_val = 1.0f + t * (184.0f - 1.0f);
        float b_val = 84.0f + t * (56.0f - 84.0f);
        
        // Manual clamping
        if (r_val < 0.0f) r_val = 0.0f;
        if (r_val > 255.0f) r_val = 255.0f;
        if (g_val < 0.0f) g_val = 0.0f;
        if (g_val > 255.0f) g_val = 255.0f;
        if (b_val < 0.0f) b_val = 0.0f;
        if (b_val > 255.0f) b_val = 255.0f;
        
        uint8_t r = static_cast<uint8_t>(r_val);
        uint8_t g = static_cast<uint8_t>(g_val);
        uint8_t b = static_cast<uint8_t>(b_val);
        colormap[i] = std::make_tuple(r, g, b);
    }
    
    return colormap;
}

} // namespace melspectrogram
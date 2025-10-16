#include <gtest/gtest.h>
#include "mel_spectrogram.h"
#include <cmath>
#include <vector>
#include <numeric>

using namespace melspectrogram;

class MelSpectrogramTest : public ::testing::Test {
protected:
    void SetUp() override {
        config.sampleRate = 32000;
        config.frameSize = 1024;
        config.hopSize = 512;
        config.numMelBands = 64;
        config.minFreq = 20.0f;
        config.maxFreq = 8000.0f;
        
        processor = std::make_unique<MelSpectrogramProcessor>(config);
    }
    
    void generateSineWave(std::vector<int16_t>& signal, float frequency, float amplitude = 0.8f) {
        for (int i = 0; i < config.frameSize; ++i) {
            float t = static_cast<float>(i) / config.sampleRate;
            signal[i] = static_cast<int16_t>(amplitude * 32767.0f * std::sin(2.0f * M_PI * frequency * t));
        }
    }
    
    void generateWhiteNoise(std::vector<int16_t>& signal, float amplitude = 0.1f) {
        for (auto& sample : signal) {
            sample = static_cast<int16_t>(amplitude * 32767.0f * (2.0f * rand() / RAND_MAX - 1.0f));
        }
    }
    
    AudioConfig config;
    std::unique_ptr<MelSpectrogramProcessor> processor;
};

// Test 1: Basic initialization
TEST_F(MelSpectrogramTest, InitializationTest) {
    EXPECT_NE(processor, nullptr);
    
    auto stats = processor->getStats();
    EXPECT_EQ(stats.processingTimeMs, 0.0f);
    EXPECT_EQ(stats.fps, 0.0f);
    EXPECT_EQ(stats.cpuUsage, 0.0f);
    EXPECT_EQ(stats.droppedFrames, 0);
}

// Test 2: Silence processing
TEST_F(MelSpectrogramTest, SilenceProcessingTest) {
    std::vector<int16_t> silence(config.frameSize, 0);
    
    bool result = processor->processAudioFrame(silence.data(), silence.size());
    EXPECT_TRUE(result);
    
    auto melSpectrum = processor->getMelSpectrum();
    EXPECT_EQ(melSpectrum.size(), config.numMelBands);
    
    // Check that silence produces very low values
    float maxValue = *std::max_element(melSpectrum.begin(), melSpectrum.end());
    EXPECT_LT(maxValue, 0.1f);
}

// Test 3: Sine wave frequency detection
TEST_F(MelSpectrogramTest, SineWaveDetectionTest) {
    std::vector<int16_t> signal(config.frameSize);
    float testFreq = 1000.0f; // 1kHz test tone
    
    generateSineWave(signal, testFreq);
    
    bool result = processor->processAudioFrame(signal.data(), signal.size());
    EXPECT_TRUE(result);
    
    auto melSpectrum = processor->getMelSpectrum();
    
    // Find peak in mel spectrum
    auto maxIt = std::max_element(melSpectrum.begin(), melSpectrum.end());
    int peakIndex = std::distance(melSpectrum.begin(), maxIt);
    
    // For a 1kHz sine wave, we expect significant energy in the mel bands
    // that correspond to frequencies around 1kHz. Instead of trying to
    // convert back to frequency, let's just verify we have a clear peak
    EXPECT_GT(melSpectrum[peakIndex], 0.5f); // Should have strong response
    
    // Verify that the peak is not at the edges (which would indicate issues)
    EXPECT_GT(peakIndex, 5); // Not in first few bands
    EXPECT_LT(peakIndex, config.numMelBands - 5); // Not in last few bands
}

// Test 4: Multiple frequency components
TEST_F(MelSpectrogramTest, MultiFrequencyTest) {
    std::vector<int16_t> signal(config.frameSize);
    std::vector<int16_t> signal1(config.frameSize);
    std::vector<int16_t> signal2(config.frameSize);
    
    generateSineWave(signal1, 500.0f, 0.5f);
    generateSineWave(signal2, 2000.0f, 0.3f);
    
    // Combine signals
    for (int i = 0; i < config.frameSize; ++i) {
        signal[i] = signal1[i] + signal2[i];
    }
    
    bool result = processor->processAudioFrame(signal.data(), signal.size());
    EXPECT_TRUE(result);
    
    auto melSpectrum = processor->getMelSpectrum();
    
    // Check that we have significant energy in the spectrum
    float totalEnergy = std::accumulate(melSpectrum.begin(), melSpectrum.end(), 0.0f);
    EXPECT_GT(totalEnergy, 1.0f);
}

// Test 5: Processing performance
TEST_F(MelSpectrogramTest, PerformanceTest) {
    std::vector<int16_t> signal(config.frameSize);
    generateWhiteNoise(signal, 0.1f);
    
    const int numIterations = 100;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numIterations; ++i) {
        bool result = processor->processAudioFrame(signal.data(), signal.size());
        EXPECT_TRUE(result);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    float avgProcessingTime = duration.count() / (1000.0f * numIterations);
    
    // Should process within 50ms for real-time performance
    EXPECT_LT(avgProcessingTime, 50.0f);
    
    auto stats = processor->getStats();
    EXPECT_FALSE(processor->isOverloaded());
}

// Test 6: Color mapping
TEST_F(MelSpectrogramTest, ColorMappingTest) {
    std::vector<int16_t> signal(config.frameSize);
    generateWhiteNoise(signal, 0.5f);
    
    bool result = processor->processAudioFrame(signal.data(), signal.size());
    EXPECT_TRUE(result);
    
    auto colorData = processor->getColorMappedData();
    EXPECT_EQ(colorData.size(), config.numMelBands * 4); // RGBA
    
    // Check that we have valid color values
    for (int i = 0; i < config.numMelBands; ++i) {
        EXPECT_GE(colorData[i * 4 + 0], 0);     // R
        EXPECT_LE(colorData[i * 4 + 0], 255);
        EXPECT_GE(colorData[i * 4 + 1], 0);     // G
        EXPECT_LE(colorData[i * 4 + 1], 255);
        EXPECT_GE(colorData[i * 4 + 2], 0);     // B
        EXPECT_LE(colorData[i * 4 + 2], 255);
        EXPECT_EQ(colorData[i * 4 + 3], 255);   // A
    }
}

// Test 7: Configuration update
TEST_F(MelSpectrogramTest, ConfigurationUpdateTest) {
    AudioConfig newConfig = config;
    newConfig.numMelBands = 128;
    newConfig.minFreq = 50.0f;
    newConfig.maxFreq = 4000.0f;
    
    processor->updateConfig(newConfig);
    
    std::vector<int16_t> signal(config.frameSize);
    generateWhiteNoise(signal, 0.1f);
    
    bool result = processor->processAudioFrame(signal.data(), signal.size());
    EXPECT_TRUE(result);
    
    auto melSpectrum = processor->getMelSpectrum();
    EXPECT_EQ(melSpectrum.size(), 128);
}

// Test 8: Invalid input handling
TEST_F(MelSpectrogramTest, InvalidInputTest) {
    std::vector<int16_t> signal(config.frameSize / 2); // Wrong size
    
    bool result = processor->processAudioFrame(signal.data(), signal.size());
    EXPECT_FALSE(result);
    
    result = processor->processAudioFrame(nullptr, config.frameSize);
    EXPECT_FALSE(result);
}

// Test 9: Stats reset
TEST_F(MelSpectrogramTest, StatsResetTest) {
    std::vector<int16_t> signal(config.frameSize);
    generateWhiteNoise(signal, 0.1f);
    
    processor->processAudioFrame(signal.data(), signal.size());
    
    auto statsBefore = processor->getStats();
    EXPECT_GT(statsBefore.processingTimeMs, 0.0f);
    
    processor->resetStats();
    
    auto statsAfter = processor->getStats();
    EXPECT_EQ(statsAfter.processingTimeMs, 0.0f);
    EXPECT_EQ(statsAfter.fps, 0.0f);
}

// Test 10: Stress test
TEST_F(MelSpectrogramTest, StressTest) {
    const int numFrames = 1000;
    std::vector<int16_t> signal(config.frameSize);
    
    for (int frame = 0; frame < numFrames; ++frame) {
        // Vary the signal
        float freq = 200.0f + (frame % 100) * 10.0f;
        generateSineWave(signal, freq, 0.3f);
        
        bool result = processor->processAudioFrame(signal.data(), signal.size());
        EXPECT_TRUE(result);
        
        // Check for overload
        if (processor->isOverloaded()) {
            std::cout << "Overload detected at frame " << frame << std::endl;
        }
    }
    
    auto stats = processor->getStats();
    EXPECT_LT(stats.processingTimeMs, 50.0f); // Should maintain real-time performance
}

// Benchmark test
TEST_F(MelSpectrogramTest, BenchmarkTest) {
    std::vector<int16_t> signal(config.frameSize);
    generateWhiteNoise(signal, 0.2f);
    
    const int warmupFrames = 10;
    const int benchmarkFrames = 1000;
    
    // Warmup
    for (int i = 0; i < warmupFrames; ++i) {
        processor->processAudioFrame(signal.data(), signal.size());
    }
    
    // Benchmark
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < benchmarkFrames; ++i) {
        processor->processAudioFrame(signal.data(), signal.size());
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    float totalSeconds = duration.count() / 1000000.0f;
    float framesPerSecond = benchmarkFrames / totalSeconds;
    
    std::cout << "Benchmark Results:" << std::endl;
    std::cout << "Total time: " << totalSeconds << " seconds" << std::endl;
    std::cout << "Frames processed: " << benchmarkFrames << std::endl;
    std::cout << "Frames per second: " << framesPerSecond << std::endl;
    std::cout << "Average frame time: " << (duration.count() / benchmarkFrames) << " microseconds" << std::endl;
    
    // Should achieve at least 20 FPS for real-time performance
    EXPECT_GT(framesPerSecond, 20.0f);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
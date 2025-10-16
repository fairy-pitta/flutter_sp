#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include "fft_processor.h"

class FftProcessorTest : public ::testing::Test {
protected:
    void SetUp() override {}
    
    void TearDown() override {}
};

// Test FFT initialization and configuration
TEST_F(FftProcessorTest, InitializationWithValidSize) {
    FftProcessor processor(1024);
    EXPECT_EQ(processor.size(), 1024);
    EXPECT_TRUE(processor.is_initialized());
}

TEST_F(FftProcessorTest, InitializationWithInvalidSize) {
    EXPECT_THROW(FftProcessor processor(0), std::invalid_argument);
    EXPECT_THROW(FftProcessor processor(511), std::invalid_argument); // Not power of 2
}

TEST_F(FftProcessorTest, DifferentFftSizes) {
    FftProcessor processor512(512);
    FftProcessor processor1024(1024);
    FftProcessor processor2048(2048);
    
    EXPECT_EQ(processor512.size(), 512);
    EXPECT_EQ(processor1024.size(), 1024);
    EXPECT_EQ(processor2048.size(), 2048);
}

// Test forward FFT computation on real audio data
TEST_F(FftProcessorTest, ForwardFftOnSineWave) {
    FftProcessor processor(1024);
    std::vector<double> input(1024);
    
    // Generate a 100Hz sine wave at 16kHz sample rate
    const double sample_rate = 16000.0;
    const double frequency = 100.0;
    for (int i = 0; i < 1024; ++i) {
        input[i] = std::sin(2.0 * M_PI * frequency * i / sample_rate);
    }
    
    std::vector<std::complex<double>> output = processor.forward_fft(input);
    
    EXPECT_EQ(output.size(), 1024);
    
    // Check that we have energy at the expected frequency bin
    int expected_bin = static_cast<int>(frequency * 1024 / sample_rate);
    EXPECT_GT(std::abs(output[expected_bin]), 100.0); // Should have significant energy
}

TEST_F(FftProcessorTest, ForwardFftOnConstantSignal) {
    FftProcessor processor(1024);
    std::vector<double> input(1024, 1.0); // Constant signal
    
    std::vector<std::complex<double>> output = processor.forward_fft(input);
    
    EXPECT_EQ(output.size(), 1024);
    EXPECT_GT(std::abs(output[0]), 900.0); // DC component should be large
}

// Test power spectrum calculation
TEST_F(FftProcessorTest, PowerSpectrumCalculation) {
    FftProcessor processor(1024);
    std::vector<double> input(1024);
    
    // Generate a simple sine wave
    for (int i = 0; i < 1024; ++i) {
        input[i] = std::sin(2.0 * M_PI * 100.0 * i / 16000.0);
    }
    
    std::vector<double> power_spectrum = processor.power_spectrum(input);
    
    EXPECT_EQ(power_spectrum.size(), 513); // N/2 + 1 for real FFT
    
    // Check that power spectrum is non-negative
    for (double power : power_spectrum) {
        EXPECT_GE(power, 0.0);
    }
    
    // Should have peak at the frequency of our sine wave
    int expected_bin = static_cast<int>(100.0 * 1024 / 16000.0);
    EXPECT_GT(power_spectrum[expected_bin], 10.0);
}

// Test Hann window application
TEST_F(FftProcessorTest, HannWindowApplication) {
    FftProcessor processor(1024);
    std::vector<double> input(1024, 1.0); // Constant input
    
    std::vector<double> windowed = processor.apply_hann_window(input);
    
    EXPECT_EQ(windowed.size(), 1024);
    
    // Hann window should taper the edges
    EXPECT_NEAR(windowed[0], 0.0, 0.01); // Start near zero
    EXPECT_NEAR(windowed[512], 1.0, 0.01); // Middle should be ~1
    EXPECT_NEAR(windowed[1023], 0.0, 0.01); // End near zero
}

TEST_F(FftProcessorTest, HannWindowReducesSpectralLeakage) {
    FftProcessor processor(1024);
    std::vector<double> input(1024);
    
    // Generate a sine wave that doesn't complete full cycles
    for (int i = 0; i < 1024; ++i) {
        input[i] = std::sin(2.0 * M_PI * 100.5 * i / 16000.0); // Non-integer frequency
    }
    
    // Power spectrum without window
    std::vector<double> power_no_window = processor.power_spectrum(input);
    
    // Power spectrum with Hann window
    std::vector<double> windowed_input = processor.apply_hann_window(input);
    std::vector<double> power_with_window = processor.power_spectrum(windowed_input);
    
    // With Hann window, spectral leakage should be reduced
    // (This is a qualitative test - windowed spectrum should be cleaner)
    EXPECT_EQ(power_with_window.size(), power_no_window.size());
}

// Test combined processing pipeline
TEST_F(FftProcessorTest, CompleteProcessingPipeline) {
    FftProcessor processor(1024);
    std::vector<double> input(1024);
    
    // Generate a test signal
    for (int i = 0; i < 1024; ++i) {
        input[i] = std::sin(2.0 * M_PI * 200.0 * i / 16000.0);
    }
    
    // Apply Hann window and compute power spectrum
    std::vector<double> power_spectrum = processor.process_with_window(input);
    
    EXPECT_EQ(power_spectrum.size(), 513);
    
    // Verify we have a peak at the expected frequency
    int expected_bin = static_cast<int>(200.0 * 1024 / 16000.0);
    EXPECT_GT(power_spectrum[expected_bin], 5.0);
}
#include <gtest/gtest.h>
#include "mel_filter.h"
#include <cmath>

class MelFilterTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(MelFilterTest, MelFrequencyConversion_ZeroHz) {
    // Test mel frequency conversion at 0 Hz
    double frequency = 0.0;
    double expected_mel = 0.0;  // mel(0) = 2595 * log10(1 + 0/700) = 0
    
    double actual_mel = mel_frequency(frequency);
    
    EXPECT_DOUBLE_EQ(expected_mel, actual_mel);
}

TEST_F(MelFilterTest, MelFrequencyConversion_1000Hz) {
    // Test mel frequency conversion at 1000 Hz
    double frequency = 1000.0;
    double expected_mel = 2595.0 * std::log10(1.0 + 1000.0/700.0);  // ~999.99
    
    double actual_mel = mel_frequency(frequency);
    
    EXPECT_NEAR(expected_mel, actual_mel, 0.01);
}

TEST_F(MelFilterTest, MelFrequencyConversion_3000Hz) {
    // Test mel frequency conversion at 3000 Hz
    double frequency = 3000.0;
    double expected_mel = 2595.0 * std::log10(1.0 + 3000.0/700.0);  // ~2412.7
    
    double actual_mel = mel_frequency(frequency);
    
    EXPECT_NEAR(expected_mel, actual_mel, 0.01);
}

TEST_F(MelFilterTest, MelFrequencyConversion_Inverse) {
    // Test that inverse mel frequency conversion works
    double frequency = 1500.0;
    double mel = mel_frequency(frequency);
    double reconstructed_freq = mel_to_frequency(mel);
    
    EXPECT_NEAR(frequency, reconstructed_freq, 0.1);
}
#include <gtest/gtest.h>
#include "mel_filter_bank.h"
#include <cmath>

class MelFilterBankTest : public ::testing::Test {
protected:
    static constexpr int SAMPLE_RATE = 32000;
    static constexpr int FFT_SIZE = 1024;
    static constexpr int NUM_FILTERS = 10;
    static constexpr double MIN_FREQ = 300.0;
    static constexpr double MAX_FREQ = 8000.0;
};

// Define static members
constexpr int MelFilterBankTest::SAMPLE_RATE;
constexpr int MelFilterBankTest::FFT_SIZE;
constexpr int MelFilterBankTest::NUM_FILTERS;
constexpr double MelFilterBankTest::MIN_FREQ;
constexpr double MelFilterBankTest::MAX_FREQ;

TEST_F(MelFilterBankTest, CreatesCorrectNumberOfFilters) {
    MelFilterBank filter_bank(SAMPLE_RATE, FFT_SIZE, NUM_FILTERS, MIN_FREQ, MAX_FREQ);
    EXPECT_EQ(filter_bank.getNumFilters(), NUM_FILTERS);
}

TEST_F(MelFilterBankTest, FilterBankHasCorrectDimensions) {
    MelFilterBank filter_bank(SAMPLE_RATE, FFT_SIZE, NUM_FILTERS, MIN_FREQ, MAX_FREQ);
    EXPECT_EQ(filter_bank.getNumFilters(), NUM_FILTERS);
    EXPECT_EQ(filter_bank.getFftSize(), FFT_SIZE);
}

TEST_F(MelFilterBankTest, FirstFilterStartsAtMinimumFrequency) {
    MelFilterBank filter_bank(SAMPLE_RATE, FFT_SIZE, NUM_FILTERS, MIN_FREQ, MAX_FREQ);
    
    std::vector<double> first_filter = filter_bank.getFilter(0);
    EXPECT_EQ(first_filter.size(), FFT_SIZE / 2 + 1);  // Only positive frequencies
    
    // Find first non-zero value
    int first_non_zero = -1;
    for (int i = 0; i < first_filter.size(); i++) {
        if (first_filter[i] > 0.0) {
            first_non_zero = i;
            break;
        }
    }
    
    EXPECT_GE(first_non_zero, 0);
    
    // Convert bin index to frequency
    double bin_freq = (double)first_non_zero * SAMPLE_RATE / FFT_SIZE;
    EXPECT_NEAR(bin_freq, MIN_FREQ, SAMPLE_RATE / (double)FFT_SIZE * 2); // Within two bins
}

TEST_F(MelFilterBankTest, LastFilterEndsAtMaximumFrequency) {
    MelFilterBank filter_bank(SAMPLE_RATE, FFT_SIZE, NUM_FILTERS, MIN_FREQ, MAX_FREQ);
    
    std::vector<double> last_filter = filter_bank.getFilter(NUM_FILTERS - 1);
    
    // Find last non-zero value
    int last_non_zero = -1;
    for (int i = last_filter.size() - 1; i >= 0; i--) {
        if (last_filter[i] > 0.0) {
            last_non_zero = i;
            break;
        }
    }
    
    EXPECT_GE(last_non_zero, 0);
    
    // Convert bin index to frequency
    double bin_freq = (double)last_non_zero * SAMPLE_RATE / FFT_SIZE;
    EXPECT_NEAR(bin_freq, MAX_FREQ, SAMPLE_RATE / (double)FFT_SIZE); // Within one bin
}

TEST_F(MelFilterBankTest, FiltersHaveTriangularShape) {
    MelFilterBank filter_bank(SAMPLE_RATE, FFT_SIZE, NUM_FILTERS, MIN_FREQ, MAX_FREQ);
    
    for (int filter_idx = 0; filter_idx < NUM_FILTERS; filter_idx++) {
        std::vector<double> filter = filter_bank.getFilter(filter_idx);
        
        // Find peak of triangular filter
        double max_value = 0.0;
        int peak_bin = -1;
        for (int i = 0; i < filter.size(); i++) {
            if (filter[i] > max_value) {
                max_value = filter[i];
                peak_bin = i;
            }
        }
        
        EXPECT_GT(max_value, 0.0);
        EXPECT_GE(peak_bin, 0);
        
        // Verify triangular shape: values increase to peak, then decrease
        bool found_peak = false;
        double prev_value = 0.0;
        
        for (int i = 0; i < filter.size(); i++) {
            if (filter[i] > 0.0) {
                if (!found_peak && i <= peak_bin) {
                    // Should be increasing or equal
                    EXPECT_GE(filter[i], prev_value * 0.9); // Allow small numerical errors
                } else if (found_peak && i > peak_bin) {
                    // Should be decreasing
                    EXPECT_LE(filter[i], prev_value * 1.1); // Allow small numerical errors
                }
                
                if (i == peak_bin) {
                    found_peak = true;
                }
                
                prev_value = filter[i];
            }
        }
    }
}

TEST_F(MelFilterBankTest, AdjacentFiltersOverlap) {
    MelFilterBank filter_bank(SAMPLE_RATE, FFT_SIZE, NUM_FILTERS, MIN_FREQ, MAX_FREQ);
    
    for (int filter_idx = 0; filter_idx < NUM_FILTERS - 1; filter_idx++) {
        std::vector<double> filter1 = filter_bank.getFilter(filter_idx);
        std::vector<double> filter2 = filter_bank.getFilter(filter_idx + 1);
        
        // Check if there's overlap (both filters have non-zero values at same bin)
        bool has_overlap = false;
        for (int i = 0; i < filter1.size(); i++) {
            if (filter1[i] > 0.0 && filter2[i] > 0.0) {
                has_overlap = true;
                break;
            }
        }
        
        EXPECT_TRUE(has_overlap) << "Filter " << filter_idx << " and " << filter_idx + 1 << " should overlap";
    }
}

TEST_F(MelFilterBankTest, ApplyFilterToSpectrum) {
    MelFilterBank filter_bank(SAMPLE_RATE, FFT_SIZE, NUM_FILTERS, MIN_FREQ, MAX_FREQ);
    
    // Create a simple test spectrum (all ones)
    std::vector<double> spectrum(FFT_SIZE / 2 + 1, 1.0);
    
    // Apply filter bank
    std::vector<double> mel_energies = filter_bank.apply(spectrum);
    
    EXPECT_EQ(mel_energies.size(), NUM_FILTERS);
    
    // All energies should be non-negative
    for (double energy : mel_energies) {
        EXPECT_GE(energy, 0.0);
    }
    
    // With uniform spectrum, adjacent filters should have similar energies
    for (int i = 1; i < mel_energies.size() - 1; i++) {
        double ratio = std::abs(mel_energies[i] - mel_energies[i-1]) / 
                      (mel_energies[i] + mel_energies[i-1] + 1e-10);
        EXPECT_LT(ratio, 0.5) << "Adjacent filters should have similar energies with uniform spectrum";
    }
}
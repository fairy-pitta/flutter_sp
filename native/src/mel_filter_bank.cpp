#include "mel_filter_bank.h"
#include "mel_filter.h"
#include <cmath>
#include <algorithm>

MelFilterBank::MelFilterBank(int sample_rate, int fft_size, int num_filters, 
                           double min_freq, double max_freq)
    : sample_rate_(sample_rate), fft_size_(fft_size), num_filters_(num_filters),
      min_freq_(min_freq), max_freq_(max_freq) {
    initializeFilters();
}

std::vector<double> MelFilterBank::getFilter(int filter_idx) const {
    // TODO: Return actual filter coefficients
    if (filter_idx < 0 || filter_idx >= num_filters_) {
        return std::vector<double>();
    }
    return filters_[filter_idx];
}

std::vector<double> MelFilterBank::apply(const std::vector<double>& power_spectrum) const {
    std::vector<double> mel_energies(num_filters_, 0.0);
    
    // Ensure power spectrum is the right size
    if (power_spectrum.size() != fft_size_ / 2 + 1) {
        return mel_energies;
    }
    
    // Apply each filter
    for (int filter_idx = 0; filter_idx < num_filters_; filter_idx++) {
        double energy = 0.0;
        for (int bin = 0; bin < power_spectrum.size(); bin++) {
            energy += power_spectrum[bin] * filters_[filter_idx][bin];
        }
        mel_energies[filter_idx] = energy;
    }
    
    return mel_energies;
}

std::vector<double> MelFilterBank::createTriangularFilter(double center_freq, 
                                                       double low_freq, 
                                                       double high_freq) const {
    std::vector<double> filter(fft_size_ / 2 + 1, 0.0);
    
    int low_bin = freqToBin(low_freq);
    int center_bin = freqToBin(center_freq);
    int high_bin = freqToBin(high_freq);
    
    // Ensure bins are within valid range
    low_bin = std::max(0, std::min(low_bin, fft_size_ / 2));
    center_bin = std::max(0, std::min(center_bin, fft_size_ / 2));
    high_bin = std::max(0, std::min(high_bin, fft_size_ / 2));
    
    // Create triangular filter
    for (int bin = low_bin; bin <= center_bin && bin < filter.size(); bin++) {
        if (center_bin > low_bin) {
            double slope = (bin - low_bin) / static_cast<double>(center_bin - low_bin);
            filter[bin] = slope;
        }
    }
    
    for (int bin = center_bin + 1; bin <= high_bin && bin < filter.size(); bin++) {
        if (high_bin > center_bin) {
            double slope = (high_bin - bin) / static_cast<double>(high_bin - center_bin);
            filter[bin] = slope;
        }
    }
    
    return filter;
}

int MelFilterBank::freqToBin(double freq) const {
    return static_cast<int>(std::round(freq * fft_size_ / sample_rate_));
}

double MelFilterBank::binToFreq(int bin) const {
    return bin * sample_rate_ / static_cast<double>(fft_size_);
}

void MelFilterBank::initializeFilters() {
    filters_.resize(num_filters_);
    
    // Convert frequency range to mel scale
    double min_mel = mel_frequency(min_freq_);
    double max_mel = mel_frequency(max_freq_);
    
    // Create evenly spaced mel points
    std::vector<double> mel_points;
    for (int i = 0; i <= num_filters_ + 1; i++) {
        double mel = min_mel + (max_mel - min_mel) * i / (num_filters_ + 1);
        mel_points.push_back(mel);
    }
    
    // Convert back to linear frequency and create filters
    for (int i = 0; i < num_filters_; i++) {
        double low_mel = mel_points[i];
        double center_mel = mel_points[i + 1];
        double high_mel = mel_points[i + 2];
        
        double low_freq = mel_to_frequency(low_mel);
        double center_freq = mel_to_frequency(center_mel);
        double high_freq = mel_to_frequency(high_mel);
        
        filters_[i] = createTriangularFilter(center_freq, low_freq, high_freq);
    }
}
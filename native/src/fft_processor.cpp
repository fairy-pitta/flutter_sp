#include "fft_processor.h"
#include "kiss_fft.h"
#include <cmath>
#include <algorithm>

FftProcessor::FftProcessor(int size) : size_(size), initialized_(false), kiss_fft_cfg_(nullptr) {
    if (size <= 0 || !is_power_of_two(size)) {
        throw std::invalid_argument("FFT size must be a positive power of 2");
    }
    
    initialize_fft();
    generate_hann_window();
    initialized_ = true;
}

FftProcessor::~FftProcessor() {
    cleanup_fft();
}

FftProcessor::FftProcessor(FftProcessor&& other) noexcept 
    : size_(other.size_), initialized_(other.initialized_), 
      kiss_fft_cfg_(other.kiss_fft_cfg_), hann_window_(std::move(other.hann_window_)) {
    other.kiss_fft_cfg_ = nullptr;
    other.initialized_ = false;
}

FftProcessor& FftProcessor::operator=(FftProcessor&& other) noexcept {
    if (this != &other) {
        cleanup_fft();
        
        size_ = other.size_;
        initialized_ = other.initialized_;
        kiss_fft_cfg_ = other.kiss_fft_cfg_;
        hann_window_ = std::move(other.hann_window_);
        
        other.kiss_fft_cfg_ = nullptr;
        other.initialized_ = false;
    }
    return *this;
}

bool FftProcessor::is_power_of_two(int n) const {
    if (n <= 0) return false;
    return (n & (n - 1)) == 0;
}

void FftProcessor::initialize_fft() {
    kiss_fft_cfg_ = kiss_fft_alloc(size_, 0, nullptr, nullptr);
    if (!kiss_fft_cfg_) {
        throw std::runtime_error("Failed to initialize KissFFT");
    }
}

void FftProcessor::cleanup_fft() {
    if (kiss_fft_cfg_) {
        kiss_fft_free(kiss_fft_cfg_);
        kiss_fft_cfg_ = nullptr;
    }
}

void FftProcessor::generate_hann_window() {
    hann_window_.resize(size_);
    for (int i = 0; i < size_; ++i) {
        hann_window_[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (size_ - 1)));
    }
}

std::vector<std::complex<double>> FftProcessor::forward_fft(const std::vector<double>& input) {
    if (!initialized_) {
        throw std::runtime_error("FFT processor not initialized");
    }
    
    if (input.size() != static_cast<size_t>(size_)) {
        throw std::invalid_argument("Input size must match FFT size");
    }
    
    // Prepare input for KissFFT (real to complex)
    std::vector<kiss_fft_cpx> fft_input(size_);
    std::vector<kiss_fft_cpx> fft_output(size_);
    
    for (int i = 0; i < size_; ++i) {
        fft_input[i].r = input[i];
        fft_input[i].i = 0.0;
    }
    
    // Perform FFT
    kiss_fft(reinterpret_cast<kiss_fft_cfg>(kiss_fft_cfg_), 
             fft_input.data(), fft_output.data());
    
    // Convert to complex<double>
    std::vector<std::complex<double>> result(size_);
    for (int i = 0; i < size_; ++i) {
        result[i] = std::complex<double>(fft_output[i].r, fft_output[i].i);
    }
    
    return result;
}

std::vector<double> FftProcessor::power_spectrum(const std::vector<double>& input) {
    if (!initialized_) {
        throw std::runtime_error("FFT processor not initialized");
    }
    
    if (input.size() != static_cast<size_t>(size_)) {
        throw std::invalid_argument("Input size must match FFT size");
    }
    
    // Perform FFT
    std::vector<std::complex<double>> fft_result = forward_fft(input);
    
    // Calculate power spectrum (magnitude squared)
    std::vector<double> power(size_ / 2 + 1);
    
    // DC component
    power[0] = (fft_result[0].real() * fft_result[0].real()) / size_;
    
    // Positive frequencies (excluding Nyquist)
    for (int i = 1; i < size_ / 2; ++i) {
        double real = fft_result[i].real();
        double imag = fft_result[i].imag();
        power[i] = (real * real + imag * imag) / size_;
    }
    
    // Nyquist frequency
    if (size_ % 2 == 0) {
        power[size_ / 2] = (fft_result[size_ / 2].real() * fft_result[size_ / 2].real()) / size_;
    }
    
    return power;
}

std::vector<double> FftProcessor::apply_hann_window(const std::vector<double>& input) {
    if (!initialized_) {
        throw std::runtime_error("FFT processor not initialized");
    }
    
    if (input.size() != static_cast<size_t>(size_)) {
        throw std::invalid_argument("Input size must match FFT size");
    }
    
    std::vector<double> windowed(size_);
    for (int i = 0; i < size_; ++i) {
        windowed[i] = input[i] * hann_window_[i];
    }
    
    return windowed;
}

std::vector<double> FftProcessor::process_with_window(const std::vector<double>& input) {
    if (!initialized_) {
        throw std::runtime_error("FFT processor not initialized");
    }
    
    // Apply Hann window and compute power spectrum in one step
    std::vector<double> windowed = apply_hann_window(input);
    return power_spectrum(windowed);
}
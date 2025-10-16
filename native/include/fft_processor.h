#ifndef FFT_PROCESSOR_H
#define FFT_PROCESSOR_H

#include <vector>
#include <complex>
#include <stdexcept>

class FftProcessor {
public:
    // Constructor - initializes FFT with given size (must be power of 2)
    explicit FftProcessor(int size);
    
    // Destructor
    ~FftProcessor();
    
    // Delete copy constructor and assignment operator
    FftProcessor(const FftProcessor&) = delete;
    FftProcessor& operator=(const FftProcessor&) = delete;
    
    // Move constructor and assignment operator
    FftProcessor(FftProcessor&&) noexcept;
    FftProcessor& operator=(FftProcessor&&) noexcept;
    
    // Get FFT size
    int size() const { return size_; }
    
    // Check if processor is properly initialized
    bool is_initialized() const { return initialized_; }
    
    // Forward FFT on real input data
    std::vector<std::complex<double>> forward_fft(const std::vector<double>& input);
    
    // Compute power spectrum from real input (applies FFT and calculates magnitude squared)
    std::vector<double> power_spectrum(const std::vector<double>& input);
    
    // Apply Hann window to input data
    std::vector<double> apply_hann_window(const std::vector<double>& input);
    
    // Complete processing pipeline: apply window, compute FFT, and return power spectrum
    std::vector<double> process_with_window(const std::vector<double>& input);

private:
    int size_;
    bool initialized_;
    void* kiss_fft_cfg_;  // KissFFT configuration (opaque pointer)
    std::vector<double> hann_window_;  // Pre-computed Hann window
    
    // Validate FFT size (must be power of 2)
    bool is_power_of_two(int n) const;
    
    // Initialize KissFFT
    void initialize_fft();
    
    // Cleanup KissFFT resources
    void cleanup_fft();
    
    // Generate Hann window coefficients
    void generate_hann_window();
};

#endif // FFT_PROCESSOR_H
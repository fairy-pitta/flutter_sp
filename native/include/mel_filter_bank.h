#ifndef MEL_FILTER_BANK_H
#define MEL_FILTER_BANK_H

#include <vector>

/**
 * @brief Mel filter bank for converting power spectrum to mel-scale energies
 */
class MelFilterBank {
public:
    /**
     * @brief Construct a mel filter bank
     * @param sample_rate Sample rate in Hz
     * @param fft_size FFT size
     * @param num_filters Number of mel filters
     * @param min_freq Minimum frequency in Hz
     * @param max_freq Maximum frequency in Hz
     */
    MelFilterBank(int sample_rate, int fft_size, int num_filters, 
                  double min_freq, double max_freq);
    
    /**
     * @brief Get the number of filters
     * @return Number of mel filters
     */
    int getNumFilters() const { return num_filters_; }
    
    /**
     * @brief Get the FFT size
     * @return FFT size
     */
    int getFftSize() const { return fft_size_; }
    
    /**
     * @brief Get a specific filter
     * @param filter_idx Filter index (0 to num_filters-1)
     * @return Filter coefficients for each FFT bin
     */
    std::vector<double> getFilter(int filter_idx) const;
    
    /**
     * @brief Apply filter bank to power spectrum
     * @param power_spectrum Power spectrum (magnitude squared)
     * @return Mel-scale energies for each filter
     */
    std::vector<double> apply(const std::vector<double>& power_spectrum) const;

private:
    int sample_rate_;
    int fft_size_;
    int num_filters_;
    double min_freq_;
    double max_freq_;
    
    std::vector<std::vector<double>> filters_;  // [num_filters][fft_size/2+1]
    
    /**
     * @brief Create triangular mel filter
     * @param center_freq Center frequency in Hz
     * @param low_freq Lower frequency bound in Hz
     * @param high_freq Upper frequency bound in Hz
     * @return Filter coefficients
     */
    std::vector<double> createTriangularFilter(double center_freq, 
                                              double low_freq, 
                                              double high_freq) const;
    
    /**
     * @brief Convert frequency to FFT bin index
     * @param freq Frequency in Hz
     * @return FFT bin index
     */
    int freqToBin(double freq) const;
    
    /**
     * @brief Convert FFT bin index to frequency
     * @param bin FFT bin index
     * @return Frequency in Hz
     */
    double binToFreq(int bin) const;
    
    /**
     * @brief Initialize all filters
     */
    void initializeFilters();
};

#endif // MEL_FILTER_BANK_H
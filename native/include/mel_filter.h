#ifndef MEL_FILTER_H
#define MEL_FILTER_H

/**
 * @brief Convert frequency in Hz to mel scale
 * @param frequency Frequency in Hz
 * @return Frequency in mel scale
 */
double mel_frequency(double frequency);

/**
 * @brief Convert mel scale to frequency in Hz
 * @param mel Frequency in mel scale
 * @return Frequency in Hz
 */
double mel_to_frequency(double mel);

#endif // MEL_FILTER_H
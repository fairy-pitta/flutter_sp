#include "mel_filter.h"
#include <cmath>

double mel_frequency(double frequency) {
    return 2595.0 * std::log10(1.0 + frequency / 700.0);
}

double mel_to_frequency(double mel) {
    return 700.0 * (std::pow(10.0, mel / 2595.0) - 1.0);
}
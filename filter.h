#include <stdlib.h>

// Using an FFT, remove all frequencies below min_freq and above max_freq
//   sample:      input array of length sample_size
//   output:      output array of length sample_size/2+1
//   sample_size: the length of the sample array
//   sample_rate: the sampling rate (in Hz) of the original sample
//   min_freq:    
//   max_freq:
void band_pass (double* sample, double* output, size_t sample_size, double sample_rate, double min_freq, double max_freq);
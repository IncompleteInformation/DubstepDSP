#include <fftw3.h>

// Perform a fast fourier transform on sample, storing the result in fft
//   sample:      input array of length sample_size
//   fft:         output array of length sample_size/2+1
//   sample_size: the length of the sample array
void calc_fft (double* sample, fftw_complex* fft, size_t sample_size);

// Compute the magnitude of each bin of fft, storing the result in fft_mag
//   fft:         input array of length sample_size/2+1
//   fft_mag:     output array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
void calc_fft_mag (fftw_complex* fft, double* fft_mag, size_t sample_size);

// Return the dominant frequency in fft
//   fft:         input array of length sample_size/2+1
//   fft_mag:     input array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
//   sample_rate: the sampling rate (in Hz) of the original sample
double dominant_freq (fftw_complex* fft, double* fft_mag, size_t sample_size, double sample_rate);

// Return the dominant frequency in fft
//   fft_mag:     output array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
//   sample_rate: the sampling rate (in Hz) of the original sample
double calc_spectral_centroid(double* fft_mag, size_t sample_size, double sample_rate);

// Return the dominant frequency in fft below a given frequency
//   fft:         input array of length sample_size/2+1
//   fft_mag:     input array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
//   sample_rate: the sampling rate (in Hz) of the original sample
//   frequency:   the lowpass cutoff
double dominant_freq_lp (fftw_complex* fft, double* fft_mag, size_t sample_size, double sample_rate, int frequency);

// Return the average amplitude (volume) of the input signal
//   fft_mag:     input array of length sample_size/2+1
//   sample_size: the length of the sample array
//   sample_rate: the sampling rate (in Hz) of the original sample
//   low:         minimum frequency to be analyzed (inclusive)
//   high:        highest frequency to be analyzed (inclusive)
double calc_avg_amplitude (double* fft_mag, size_t sample_size, size_t sample_rate, size_t low, size_t high);

// Return the proportion of the energy of the dominant frequency to the rest of the signal
//   fft_mag:     input array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
//   sample_rate: the sampling rate (in Hz) of the original sample
double calc_spectral_crest(double* fft_mag, size_t sample_size, double sample_rate);

// Return a number representing spectral flatness. Geometric mean/arithmetic mean
//   fft_mag:     input array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
//   sample_rate: the sampling rate (in Hz) of the original sample
//   low:         minimum frequency to be analyzed (inclusive)
//   high:        highest frequency to be analyzed (inclusive)
double calc_spectral_flatness(double* fft_mag, size_t sample_size, size_t sample_rate, size_t low, size_t high);

// Return the average difference between harmonics between 500 and 5000Hz
//   fft:         input array of length sample_size/2+1
//   fft_mag:     input array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
//   sample_rate: the sampling rate (in Hz) of the original sample
double calc_harmonics(fftw_complex* fft, double* fft_mag, size_t sample_size, double sample_rate);

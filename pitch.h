#include <fftw3.h>

// Perform a fast fourier transform on sample, storing the result in fft
//   sample:      input array of length sample_size
//   fft:         output array of length sample_size/2+1
//   tmp:         temporary storage array of length sample_size
//   sample_size: the length of the sample array
void calc_fft (double* sample, fftw_complex* fft, double* tmp, size_t sample_size);

// Compute the magnitude of each bin of fft, storing the result in fft_mag
//   fft:         input array of length sample_size/2+1
//   fft_mag:     output array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
void calc_fft_mag (fftw_complex* fft, double* fft_mag, size_t sample_size);

// Return the dominant frequency in fft
//   fft:         input array of length sample_size/2+1
//   fft_mag:     output array of length sample_size/2+1
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
//   fft_mag:     output array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
//   sample_rate: the sampling rate (in Hz) of the original sample
//   frequency:   the lowpass cutoff
double dominant_freq_lp (fftw_complex* fft, double* fft_mag, size_t sample_size, double sample_rate, int frequency);
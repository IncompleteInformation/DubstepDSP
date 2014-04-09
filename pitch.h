#include <fftw3.h>

// Perform a fast fourier transform on sample, storing the result in fft
//   sample:      input array of length sample_size
//   fft:         output array of length sample_size/2+1
//   tmp:         temporary storage array of length sample_size
//   sample_size: the length of the sample array
void calc_fft (double* sample, fftw_complex* fft, double* tmp, size_t sample_size);

// Calculate the magnitude of each bin of fft, storing the result in fft_mag
//   fft:         input array of length sample_size/2+1
//   fft_mag:     output array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
void calc_fft_mag (fftw_complex* fft, double* fft_mag, size_t sample_size);

// TODO: Document me!
double dominant (fftw_complex* fft, size_t fft_size);
#include <fftw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Perform a fast fourier transform on sample, storing the result in fft
//   sample:      input array of length sample_size
//   fft:         output array of length sample_size/2+1
//   tmp:         temporary storage array of length sample_size
//   sample_size: the length of the sample array
void calc_fft (double* sample, fftw_complex* fft, double* tmp, size_t sample_size)
{
    for (size_t i = 0; i < sample_size; ++i) tmp[i] = sample[i];
    fftw_plan plan_forward = fftw_plan_dft_r2c_1d(sample_size, tmp, fft, FFTW_ESTIMATE);
    fftw_execute (plan_forward);
    fftw_destroy_plan(plan_forward);
}

// Calculate the magnitude of each bin of fft, storing the result in fft_mag
//   fft:         input array of length sample_size/2+1
//   fft_mag:     output array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
void calc_fft_mag (fftw_complex* fft, double* fft_mag, size_t sample_size)
{
    for (size_t i = 0; i < sample_size/2+1; ++i)
        fft_mag[i] = fft[i][0]*fft[i][0] + fft[i][1]*fft[i][1];
}

double dominant (fftw_complex* fft, size_t fft_size)
{
    return 0;
}

// Test pitch detection algorithms against a sine wave
void sine_test (size_t sample_rate, size_t sample_size, double sample_freq)
{
    // Set up
    printf("sine_test(%ld,%ld,%f)\n", sample_rate, sample_size, sample_freq);
    double* sample = malloc(sizeof(double)*sample_size);
    fftw_complex* fft = malloc(sizeof(fftw_complex)*(sample_size/2+1));
    double* fft_tmp = malloc(sizeof(double)*sample_size);
    double* fft_mag = malloc(sizeof(double)*(sample_size/2+1));

    // Generate sine wave
    for (size_t i = 0; i < sample_size; ++i)
        sample[i] = sin(2.f*M_PI*sample_freq*i/sample_rate);

    // Test functions
    calc_fft(sample, fft, fft_tmp, sample_size);
    calc_fft_mag(fft, fft_mag, sample_size);
    printf("    dominant: %f\n", dominant(fft, sample_size));

    // Clean up
    free(sample);
    free(fft);
    free(fft_tmp);
    free(fft_mag);
    printf("\n");
}

int main (void)
{
    sine_test(44100, 1024, 440);
}
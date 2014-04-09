#include "pitch.h"

#include <fftw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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
}

int main (void)
{
    sine_test(44100, 1024, 440);
}
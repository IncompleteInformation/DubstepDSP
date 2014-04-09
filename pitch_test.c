#include "pitch.h"

#include <fftw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Test pitch detection algorithms against a sine wave
void sine_test (double sample_rate, size_t sample_size, double sample_freq)
{
    // Set up
    printf("SINE TEST\n");
    printf("Sample size: %ld\n", sample_size);
    printf("Sample rate: %f\n", sample_rate);
    printf("Frequency:   %f\n", sample_freq);
    printf("Periods:     %f\n", sample_freq * sample_size / sample_rate);
    double err;
    double* sample = malloc(sizeof(double)*sample_size);
    fftw_complex* fft = malloc(sizeof(fftw_complex)*(sample_size/2+1));
    double* fft_tmp = malloc(sizeof(double)*sample_size);
    double* fft_mag = malloc(sizeof(double)*(sample_size/2+1));

    // Generate sine wave
    for (size_t i = 0; i < sample_size; ++i)
        sample[i] = sin(2.f*M_PI*sample_freq*i/sample_rate+M_PI);

    // Test functions
    calc_fft(sample, fft, fft_tmp, sample_size);
    calc_fft_mag(fft, fft_mag, sample_size);
    double dom = dominant_freq(fft, fft_mag, sample_size, sample_rate);
    err = dom - sample_freq;
    printf("    Dominant = %f (%+.2fhz)\n", dom, err);

    // Clean up
    free(sample);
    free(fft);
    free(fft_tmp);
    free(fft_mag);
    printf("\n");
}

int main (void)
{
    // Remember remember the off by 10
    sine_test(44100, 1024, 60);
    sine_test(44100, 1024, 80);
    sine_test(44100, 1024, 110);
    sine_test(44100, 1024, 120);
    sine_test(44100, 1024, 130);
    sine_test(44100, 1024, 220);
    sine_test(44100, 1024, 440);
    sine_test(44100, 1024, 450);
    sine_test(44100, 1024, 460);
    sine_test(44100, 1024, 470);
    sine_test(44100, 1024, 480);
    sine_test(44100, 1024, 490);
}
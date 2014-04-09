#include "pitch.h"

#include <fftw3.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Test pitch detection algorithms against a sine wave
bool sine_test (double sample_rate, size_t sample_size, double sample_freq)
{
    // Set up
    bool pass = true;
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
    double dom_err = dom - sample_freq;

    if (fabs(dom_err)/sample_freq > .0125)
    {
        printf("FAILED SINE TEST:\n");
        printf("    Sample size: %ld\n", sample_size);
        printf("    Sample rate: %.0f\n", sample_rate);
        printf("    Frequency:   %.0f\n", sample_freq);
        printf("    Periods:     %.2f\n", sample_freq*sample_size/sample_rate);
        printf("    Dominant = %f (%+.2fhz) (%.1f%%)\n", dom, dom_err, 100*fabs(dom_err)/sample_freq);
        pass = false;
        printf("\n");
    }

    // Clean up
    free(sample);
    free(fft);
    free(fft_tmp);
    free(fft_mag);
    return pass;
}

int main (void)
{
    for (int f = 80; f < 1200; f += 1)
        if (!sine_test(44100, 1024, f)) break;
}
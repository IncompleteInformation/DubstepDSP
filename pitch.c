#include <fftw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void calc_fft (double* sample, fftw_complex* fft, double* tmp, size_t sample_size)
{
    for (size_t i = 0; i < sample_size; ++i) tmp[i] = sample[i];
    fftw_plan plan_forward = fftw_plan_dft_r2c_1d(sample_size, tmp, fft, FFTW_ESTIMATE);
    fftw_execute (plan_forward);
    fftw_destroy_plan(plan_forward);
}

void calc_fft_mag (fftw_complex* fft, double* fft_mag, size_t sample_size)
{
    for (size_t i = 0; i < sample_size/2+1; ++i)
        fft_mag[i] = fft[i][0]*fft[i][0] + fft[i][1]*fft[i][1];
}

double dominant_freq (fftw_complex* fft, double* fft_mag, size_t sample_size, double sample_rate)
{
    double max = 0;
    int max_bin = 0;
    for (size_t i = 0; i < sample_size/2+1; ++i)
    {
        if (fft_mag[i] > max)
        {
            max = fft_mag[i];
            max_bin = i + 1;
        }
    }

    double peak  = fft[max_bin][0];
    double left  = fft[max_bin-1][0];
    double right = fft[max_bin+1][0];
    double delta = (right - left) / (2 * peak - left - right);
    printf("%f\n", sample_rate / sample_size * (max_bin - delta));
    return sample_rate / sample_size * (max_bin - delta);
}

double calc_spectral_centroid(double* fft_mag, size_t sample_size, double sample_rate)
{
    double top_sum = 0;
    double bottom_sum = 0;
    for (int i=0; i<sample_size/2 + 1; ++i)
    {
        // spectral magnitude is already contained in fft_result
        bottom_sum+=fft_mag[i];
        top_sum+=fft_mag[i]*(i+1)*(sample_rate/sample_size);
    }
    double spectral_centroid = top_sum/bottom_sum;
    return spectral_centroid;
}
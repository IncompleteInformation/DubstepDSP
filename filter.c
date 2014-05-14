#include <fftw3.h>

void calc_fft (double* sample, double* output, size_t sample_size, double sample_rate, double min_freq, double max_freq)
{
    // Take FFT
    fftw_complex* fft = fftw_malloc(sizeof(fftw_complex) * sample_size);
    fftw_plan plan_forward = fftw_plan_dft_r2c_1d((int)sample_size, sample, fft, FFTW_ESTIMATE);
    fftw_execute(plan_forward);

    // Apply band pass
    int min_bin = min_freq * sample_size / sample_rate;
    int max_bin = max_freq * sample_size / sample_rate;
    for (int i = 0; i < sample_size; ++i)
    {
        if (i < min_bin || max_bin < i)
        {
            fft[i][0] = 0;
            fft[i][1] = 0;
        }
    }

    // Take inverse
    fftw_plan plan_inverse = fftw_plan_dft_c2r_1d((int)sample_size, fft, output, FFTW_ESTIMATE);
    fftw_execute(plan_inverse);

    // Normalize output
    for (int i = 0; i < sample_size; ++i)
    {
        output[i] /= sample_size;
    }

    // Clean up
    fftw_destroy_plan(plan_inverse);
    fftw_destroy_plan(plan_forward);
    fftw_free(fft);
}
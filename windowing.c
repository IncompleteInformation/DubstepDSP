#include <math.h>
#include <stdlib.h>

#define PI 3.14159265

void welch_window(double* fft_mag, size_t sample_size, double* windowed)
{
    double N = (double)sample_size;
    for (size_t n = 0; n < sample_size; ++n)
    {
        double coeff = (n-(N-1)/2)/((N+1)/2);
        windowed[n] = (1 - coeff*coeff) * fft_mag[n];
    }
}

void hanning_window(double* fft_mag, size_t sample_size, double* windowed)
{
    double N = (double)sample_size;
    for (size_t n = 0; n < sample_size; ++n)
    {
        double coeff = 0.5 * (1 + cos((2*PI*n)/(N-1)));
        windowed[n] = coeff * fft_mag[n];
    }
}

void hamming_window(double* fft_mag, size_t sample_size, double* windowed)
{
    double N = (double)sample_size;
    for (size_t n = 0; n < sample_size; ++n)
    {
        
        double coeff = 0.54 + (0.46 * cos((2*PI*n)/(N-1)));
        windowed[n] = coeff * fft_mag[n];
    }
}

void blackman_window(double* fft_mag, size_t sample_size, double* windowed)
{
    double N = (double)sample_size;
    double alpha = 0.16;
    double a_0 = (1-alpha)/2;
    double a_1 = 0.5;
    double a_2 = alpha/2;
    for (size_t n = 0; n < sample_size; ++n)
    {
        double coeff = a_0 - a_1*cos((2*PI*n)/(N-1)) + a_2*cos((4*PI*n)/(N-1));
        windowed[n] = coeff * fft_mag[n];
    }
}

void nuttal_window(double* fft_mag, size_t sample_size, double* windowed)
{
    double N = (double)sample_size;
    double a_0 = 0.355768;
    double a_1 = 0.487396;
    double a_2 = 0.144232;
    double a_3 = 0.012604;
    for (size_t n = 0; n < sample_size; ++n)
    {
        double coeff = a_0 - a_1*cos((2*PI*n)/(N-1)) + a_2*cos((4*PI*n)/(N-1)) - a_3*cos((6*PI*n)/(N-1));
        windowed[n] = coeff * fft_mag[n];
    }
}

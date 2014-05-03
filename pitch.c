#include <fftw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define E 2.71828182845904523536028747135266249775724709369995


void calc_fft (double* sample, fftw_complex* fft, double* tmp, size_t sample_size)
{
    for (size_t i = 0; i < sample_size; ++i) tmp[i] = sample[i];
    fftw_plan plan_forward = fftw_plan_dft_r2c_1d((int)sample_size, tmp, fft, FFTW_ESTIMATE);
    fftw_execute (plan_forward);
    fftw_destroy_plan(plan_forward);
}

void calc_fft_mag (fftw_complex* fft, double* fft_mag, size_t sample_size)
{
    for (size_t i = 0; i < sample_size/2+1; ++i)
        fft_mag[i] = (fft[i][0]*fft[i][0] + fft[i][1]*fft[i][1])/((sample_size/2)*(sample_size/2));
}

double dominant_freq (fftw_complex* fft, double* fft_mag, size_t sample_size, double sample_rate)
{
    double max = 0;
    long max_bin = 0;
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
    return sample_rate / sample_size * (max_bin - delta);
}

double dominant_freq_lp (fftw_complex* fft, double* fft_mag, size_t sample_size, double sample_rate, int frequency)
{
    double bin_size = sample_rate/sample_size;
    double max = 0;
    long max_bin = 0;
    size_t num_bins   = frequency/bin_size + 1;
    for (size_t i = 0; i<num_bins; ++i)
    {
        if (fft_mag[i]> max)
        {
            max = fft_mag[i];
            max_bin = i;
        }
    }
    double normalized[num_bins];
    for (size_t i = 0; i<num_bins; ++i)
    {
        normalized[i] = fft_mag[i]/max;
    }
    double threshold = .0625;
    for (size_t i = 1; i<num_bins; ++i)
    {
        if ((normalized[i]>normalized[i-1]) && (normalized[i]>threshold))
        {
            if (normalized[i+1]>normalized[i])
            {
                max_bin = i+1;
            }
            else
            {
                max_bin = i;
                break;
            }
        }
    }

    double peak  = fft[max_bin][0];
    double left  = fft[max_bin-1][0];
    double right = fft[max_bin+1][0];
    double delta = (right - left) / (2 * peak - left - right);

    double candidate = sample_rate / sample_size * (max_bin - delta);
    if (candidate > 50) return candidate;
    else return -INFINITY;
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

double calc_avg_amplitude (double* fft_mag, size_t sample_size, size_t sample_rate, size_t low, size_t high)
{
    double average = 0;
    size_t bin_size = sample_rate/sample_size;
    size_t start = low/bin_size;
    size_t end   = high/bin_size + 1;
    size_t num_bins = end-start;
    for (size_t i = start; i < end; ++i)
    {
        average += fft_mag[i];
    }
    average = average/num_bins;
    return average;
}
double calc_spectral_flatness(double* fft_mag, size_t sample_size, size_t sample_rate, size_t low, size_t high)
{
    double geo_average = 0;
    double ari_average = 0;
    double geo_total = 0;
    double ari_total = 0;
    double bin_size = sample_rate/sample_size;
    size_t start = low/bin_size;
    size_t end   = high/bin_size + 1;
    size_t num_bins = end-start;
    for (size_t i = start; i < end; ++i)
    {
        if (fft_mag[i]!=0) geo_total+=log(fft_mag[i]);
        ari_total+=fft_mag[i];
    }
    geo_average = pow(E, (double)1/num_bins)*geo_total;
    ari_average = ari_total/num_bins;
    double flatness = geo_average/ari_average;
    return flatness;
}

double calc_spectral_crest(double* fft_mag, size_t sample_size, double sample_rate)
{
    double max = 0;
    size_t maxi;
    double total = 0;
    for (int i = 0; i < sample_size/2 + 1; ++i)
    {
        total += fft_mag[i];
        if (fft_mag[i] > max)
        {
            max = fft_mag[i];
            maxi = i+1;
        }
    }
    double crest = (max*sample_size)/total;
    return crest;
}

double calc_harmonics(fftw_complex* fft, double* fft_mag, size_t sample_size, double sample_rate)
{
    double harmonics[sample_size/2+1];
    harmonics[0] = -INFINITY;
    harmonics[sample_size/2] = -INFINITY;
    double peak;
    double left;
    double right;
    double delta;
    double est_freq;
    //calc harmonics
    for (size_t i = 1; i<sample_size/2; ++i)
    {
        if ((fft_mag[i]>fft_mag[i-1])&&(fft_mag[i]>fft_mag[i+1]))
        {
            peak  = fft[i][0];
            left  = fft[i-1][0];
            right = fft[i+1][0];
            delta = (right - left) / (2 * peak - left - right);
            est_freq = sample_rate / sample_size * (i - delta);
            harmonics[i] = est_freq;
        }else{
            harmonics[i]=-INFINITY;
        }
    }
    //calc average
    double prev_harmonic = 0;
    double harmonic_total = 0;
    int    num_harmonics_seen = 0;
    for (size_t i = 0; i<sample_size/2+1; ++i)
    {
        if ((harmonics[i]>50)&&(harmonics[i]<500))
        {
            if (prev_harmonic==0) {
                prev_harmonic = harmonics[i];
                num_harmonics_seen=1;
            }else{
                harmonic_total+=(harmonics[i]-prev_harmonic);
                prev_harmonic = harmonics[i];
                num_harmonics_seen+=1;
            }
        }
    }
    return harmonic_total/num_harmonics_seen;
}
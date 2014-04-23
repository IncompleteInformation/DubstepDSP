#include "pitch.h"

#include <fftw3.h>

#define SAMPLE_RATE       44100.0
#define FRAMES_PER_BUFFER 64
#define FFT_SIZE          1024 // 1024 = 23ms delay, 43Hz bins
#define BIN_SIZE          (SAMPLE_RATE/FFT_SIZE)
#define ONSET_FFT_SIZE    64
#define ONSET_THRESHOLD   0.025

// FFT data
extern double       fft_buffer[FFT_SIZE];
extern int          fft_buffer_loc;
extern fftw_complex fft[FFT_SIZE/2 + 1];
extern double       fft_mag[FFT_SIZE/2 + 1];
extern fftw_complex fft_fft[(FFT_SIZE/2 +1)/2 +1];
extern double       fft_fft_mag[(FFT_SIZE/2+1)/2+1];
extern double       harmonics[FFT_SIZE/2+1];

// FFT characteristics
extern double       spectral_centroid;
extern double       dominant_frequency;
extern double       dominant_frequency_lp;
extern double       average_amplitude;
extern double       spectral_crest;
extern double       spectral_flatness;
extern double       harmonic_average;

// Onset detection
extern double       onset_fft_buffer[ONSET_FFT_SIZE];
extern fftw_complex onset_fft[ONSET_FFT_SIZE];
extern double       onset_fft_mag[ONSET_FFT_SIZE/2+1];
extern double       onset_average_amplitude;
extern int          onset_fft_buffer_loc;
extern int          onset_triggered;

// Initialize live system
void live_init ();

// Advance system state by a single sample
void live_push_sample (float sample);
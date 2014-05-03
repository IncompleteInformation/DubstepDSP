#define RECTANGLE   0
#define WELCH       1
#define HANNING     2
#define HAMMING     3
#define BLACKMAN    4
#define NUTTAL      5

// Return the welch-windowed version of the fft_mag buffer
//   fft_mag:     input array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
void welch_window(double* fft_mag, size_t sample_size, double* windowed);

// Return the hanning_windowed version of the fft_mag buffer
//   fft_mag:     input array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
void hanning_window(double* fft_mag, size_t sample_size, double* windowed);

// Return the hamming_windowed version of the fft_mag buffer
//   fft_mag:     input array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
void hamming_window(double* fft_mag, size_t sample_size, double* windowed);

// Return the blackman_windowed version of the fft_mag buffer
//   fft_mag:     input array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
void blackman_window(double* fft_mag, size_t sample_size, double* windowed);

// Return the nuttal_windowed version of the fft_mag buffer
//   fft_mag:     input array of length sample_size/2+1
//   sample_size: the length of the original sample the fft was based on
void nuttal_window(double* fft_mag, size_t sample_size, double* windowed);

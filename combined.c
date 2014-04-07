#include "pa_ringbuffer.h"

// #define GLFW_INCLUDE_GLCOREARB
#include <stdlib.h>
#include <stdio.h>
#include <math.h> //math comes before fftw so that fftw_complex is not overriden

#include <GLFW/glfw3.h>
#include <portaudio.h>
#include <fftw3.h>

#define SAMPLE_RATE   (44100)
#define BUFFER_SIZE   (65536)
#define FRAMES_PER_BUFFER  (64)
#define FFT_SIZE (1024) //512 = 11ms delay, 86Hz bins
#define BIN_SIZE ((double) SAMPLE_RATE/FFT_SIZE)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

float data[BUFFER_SIZE];
float bufferData[BUFFER_SIZE];
PaUtilRingBuffer buffer;

typedef struct
{
    float fft_buffer[FFT_SIZE];
    int fft_buffer_loc;
}
fftBuffer;
fftBuffer fftBuf;
double fft_result[FFT_SIZE/2 + 1];
fftw_complex fft_complex_unshifted[FFT_SIZE/2 + 1];
double spectral_centroid;
double dominant_frequency;

static void glfwError (int error, const char* description)
{
    fputs(description, stderr);
}

static void paCheckError (PaError error)
{
    if (error == paNoError) return;

    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", error );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( error ) );
    exit(EXIT_FAILURE);
}

static void onKeyPress (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}
static double abs_complex(double real, double imag)
{
    double power = pow(real,2)+pow(imag,2);
    //double normalized = 10./log(10.) * log(power + 1e-6);
    return power/FFT_SIZE; //this scaling needs fixing. some log shit, but we need to know how fftw scales on transform
}
static void calc_spectral_centroid()
/* this calculates a linear-weighted spectral centroid */
{
    double top_sum = 0;
    double bottom_sum = 0;
    for (int i=0; i<FFT_SIZE/2 + 1; ++i)
    {
        /* spectral magnitude is already contained in fft_result */
        bottom_sum+=fft_result[i];
        top_sum+=fft_result[i]*(i+1)*BIN_SIZE; 
    }
    spectral_centroid = top_sum/bottom_sum;
    //printf("%f\n", spectral_centroid);
    return;
}
static void calc_dominant_frequency()
/* this returns the Hz value of the max value bin */
{
    double max = 0;
    int max_bin_index = 0;
    for (int i=0; i<FFT_SIZE/2 + 1; ++i)
    {
        if (fft_result[i]>max){max_bin_index=(i+1); max = fft_result[i];}
    }
    double est;
    fftw_complex peak;
    peak[0] = fft_complex_unshifted[max_bin_index][0];
    peak[1] = fft_complex_unshifted[max_bin_index][1];
    fftw_complex delta;
    fftw_complex left;
    fftw_complex right;
    double peakfrac;

    if (max_bin_index==1) dominant_frequency = BIN_SIZE;
    else if (max_bin_index==FFT_SIZE) dominant_frequency = 22000; //like we care...
    else 
    {
        left[0] = fft_complex_unshifted[max_bin_index-1][0];
        left[1] = fft_complex_unshifted[max_bin_index-1][1];
        right[0] = fft_complex_unshifted[max_bin_index+1][0];
        right[0] = fft_complex_unshifted[max_bin_index+1][0];
        delta[0] = (right[0] - left[0]) / ((2 * peak[0] - left[0]) - right[0]);
        //delta[1] = (right[1] - left[1]) / ((2 * peak[1] - left[1]) - right[1]); //we only care about real part. magic. ref - jon's octave code
        peakfrac = max_bin_index - delta[0];
        est = peakfrac * BIN_SIZE;
        //dominant_frequency = max_bin_index*BIN_SIZE;
        dominant_frequency = est;
    }
    printf("%f\n", dominant_frequency);
    return;
}
static void perform_fft()
{
    int i;
    double *in;
    int nout;
    fftw_complex *out;
    fftw_plan plan_backward;
    fftw_plan plan_forward;
    /* Set up an array to hold the data, and assign the data. */
    in = fftw_malloc ( sizeof ( double ) * FFT_SIZE );
    for ( i = 0; i < FFT_SIZE; i++ ){in[i] = (double)fftBuf.fft_buffer[i];}
    nout = ( FFT_SIZE / 2 ) + 1;
    out = fftw_malloc ( sizeof ( fftw_complex ) * nout );
    plan_forward = fftw_plan_dft_r2c_1d ( FFT_SIZE, in, out, FFTW_ESTIMATE );
    fftw_execute ( plan_forward );
    for ( i = 0; i < nout; i++ )
    {
        double fft_val = abs_complex(out[i][0], out[i][1]);
        //printf ( "  %4d  %12f\n", i, fft_val );
        fft_result[i] = fft_val;
        fft_complex_unshifted[i][0] = out[i][0];
        fft_complex_unshifted[i][1] = out[i][1];
    }
    /* Release the memory associated with the plans. */
    fftw_destroy_plan ( plan_forward );
    fftw_free ( in );
    fftw_free ( out );
    return;
}
static void update_fft_buffer(float mic_bit)
{
    fftBuf.fft_buffer[fftBuf.fft_buffer_loc] = mic_bit;
    ++fftBuf.fft_buffer_loc;
    if (fftBuf.fft_buffer_loc==FFT_SIZE)
        {
            fftBuf.fft_buffer_loc=0;
            perform_fft();
            calc_spectral_centroid();
            calc_dominant_frequency();
            //for (int i=0;i<FFT_SIZE;++i){printf("%f\n", fftBuf.fft_buffer[i]);}
        }
}
static int onAudioSync (const void* inputBuffer, void* outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void* thunk)
{
    float* in = (float*)inputBuffer;
    float* out = (float*)outputBuffer;

    PaUtil_WriteRingBuffer(&buffer, in, framesPerBuffer);
    for (int i = 0; i < framesPerBuffer; ++i)
    {
        if (in[i] > 1){update_fft_buffer(1);printf("%s\n", "clipping high");continue;}
        if (in[i] < -1){update_fft_buffer(-1);printf("%s\n", "clipping low");continue;}
        update_fft_buffer(in[i]);
        // if (in[i] > 0.1) printf("%f\n", in[i]);
    }

    return paContinue;
}

int main (void)
{
    /* for loop tests expected behaviour. need to disable write in update_fft_buffer */
    for (int i = 0; i<FFT_SIZE; ++i)
    {
        fftBuf.fft_buffer[i] = sin(2*M_PI*i*((double) 20000/44100));
    }
    // Initialize ring buffer
    PaUtil_InitializeRingBuffer(&buffer, sizeof(float), BUFFER_SIZE, &bufferData);

    // Initialize PortAudio
    paCheckError(Pa_Initialize());

    // Configure stream input
    PaStreamParameters in;
    in.device = Pa_GetDefaultInputDevice();
    in.channelCount = 1;
    in.sampleFormat = paFloat32;
    in.suggestedLatency = Pa_GetDeviceInfo(in.device)->defaultLowInputLatency;
    in.hostApiSpecificStreamInfo = NULL;


    // Configure stream output
    PaStreamParameters out;
    out.device = Pa_GetDefaultOutputDevice();
    if (out.device == paNoDevice)
    {
        fprintf(stderr, "Error: No default output device.\n");
        exit(EXIT_FAILURE);
    }
    out.channelCount = 2;
    out.sampleFormat = paFloat32;
    out.suggestedLatency = Pa_GetDeviceInfo(out.device)->defaultLowOutputLatency;
    out.hostApiSpecificStreamInfo = NULL;

    // Initialize stream
    PaStream *stream;
    paCheckError(Pa_OpenStream(
                 &stream,
                 &in,
                 &out,
                 SAMPLE_RATE,
                 FRAMES_PER_BUFFER,
                 paClipOff,
                 onAudioSync,
                 NULL));

    // Initialize OpenGL window
    GLFWwindow* window;
    glfwSetErrorCallback(glfwError);
    if (!glfwInit()) exit(EXIT_FAILURE);
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, onKeyPress);

    // Play audio
    paCheckError(Pa_StartStream(stream));

    // Render shit for two seconds
    glfwSetTime(0);
    while (!glfwWindowShouldClose(window))
    {
        float aspectRatio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        aspectRatio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-aspectRatio, aspectRatio, -1.f, 1.f, 1.f, -1.f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glBegin(GL_POINTS);
        PaUtil_ReadRingBuffer(&buffer, &data, BUFFER_SIZE);
        for (int i = 0; i < BUFFER_SIZE; ++i)
        {
            glColor3f(0.0f,0.5f,0.9f);
            glVertex3f(2*aspectRatio*i/BUFFER_SIZE-aspectRatio, data[i], 0.f);
            glColor3f(1.0f,0.0f,1.0f);
            glVertex3f(2*aspectRatio*i/(FFT_SIZE/2 + 1)-aspectRatio, fft_result[i]-1, 0.f);
        }
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Shut down GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    // Shut down PortAudio
    paCheckError(Pa_StopStream(stream));
    paCheckError(Pa_CloseStream(stream));
    Pa_Terminate();

    // Exit happily
    exit(EXIT_SUCCESS);
}
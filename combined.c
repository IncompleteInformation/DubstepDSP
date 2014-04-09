#include "pa_ringbuffer.h"
#include "pitch.h"

// #define GLFW_INCLUDE_GLCOREARB
#include <stdlib.h>
#include <stdio.h>
#include <math.h> //math comes before fftw so that fftw_complex is not overriden

#include <GLFW/glfw3.h>
#include <portaudio.h>
#include <fftw3.h>

#define SAMPLE_RATE   (double)(44100)
#define BUFFER_SIZE   (65536)
#define FRAMES_PER_BUFFER  (64)
#define FFT_SIZE (1024) //512 = 11ms delay, 86Hz bins
#define BIN_SIZE ((double) SAMPLE_RATE/FFT_SIZE)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

typedef struct{
    float data[BUFFER_SIZE];
    float bufferData[BUFFER_SIZE];
    PaUtilRingBuffer buffer;

    double fft_buffer[FFT_SIZE];
    int fft_buffer_loc;
    fftw_complex fft[FFT_SIZE/2 + 1];
    double fft_mag[FFT_SIZE/2 + 1];

    double spectral_centroid;
    double dominant_frequency;
} UserData;

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

// static void calc_spectral_centroid()
// /* this calculates a linear-weighted spectral centroid */
// {
//     double top_sum = 0;
//     double bottom_sum = 0;
//     for (int i=0; i<FFT_SIZE/2 + 1; ++i)
//     {
//         // spectral magnitude is already contained in fft_result 
//         bottom_sum+=fft_result[i];
//         top_sum+=fft_result[i]*(i+1)*BIN_SIZE; 
//     }
//     spectral_centroid = top_sum/bottom_sum;
//     //printf("%f\n", spectral_centroid);
//     return;
//}
// typedef struct{
//     float data[BUFFER_SIZE];
//     float bufferData[BUFFER_SIZE];
//     PaUtilRingBuffer buffer;
//     float fft_buffer[FFT_SIZE];
//     int fft_buffer_loc;

//     double fft_mag[FFT_SIZE/2 + 1];
//     fftw_complex fft[FFT_SIZE/2 + 1];
//     double spectral_centroid;
//     double dominant_frequency;
// } userData;
static void update_fft_buffer(float mic_bit, UserData* ud)
{
    ud->fft_buffer[ud->fft_buffer_loc] = mic_bit;
    ++ud->fft_buffer_loc;
    if (ud->fft_buffer_loc==FFT_SIZE)
        {
            ud->fft_buffer_loc=0;
            double tmp[FFT_SIZE];
            calc_fft(ud->fft_buffer, ud->fft, tmp, FFT_SIZE);
            calc_fft_mag(ud->fft, ud->fft_mag, FFT_SIZE);
            ud->dominant_frequency = dominant_freq(ud->fft, ud->fft_mag, FFT_SIZE, SAMPLE_RATE);
            // calc_spectral_centroid();
        }
}
static int onAudioSync (const void* inputBuffer, void* outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void* userData)
{
    UserData* ud = (UserData*) userData;
    float* in = (float*)inputBuffer;
    float* out = (float*)outputBuffer;

    PaUtil_WriteRingBuffer(&ud->buffer, in, framesPerBuffer);
    for (int i = 0; i < framesPerBuffer; ++i)
    {
        if (in[i] > 1){update_fft_buffer(1, ud);printf("%s\n", "clipping high");continue;}
        if (in[i] < -1){update_fft_buffer(-1, ud);printf("%s\n", "clipping low");continue;}
        update_fft_buffer(in[i], ud);
        // if (in[i] > 0.1) printf("%f\n", in[i]);
    }

    return paContinue;
}

int main (void)
{
    UserData ud;


    // Initialize ring buffer
    PaUtil_InitializeRingBuffer(&ud.buffer, sizeof(float), BUFFER_SIZE, &ud.bufferData);

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
                 &ud));

    // Initialize OpenGL window
    double dbRange = 96;

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
        PaUtil_ReadRingBuffer(&ud.buffer, &ud.data, BUFFER_SIZE);
        for (int i = 0; i < BUFFER_SIZE; ++i)
        {
            glColor3f(0.0f,0.5f,0.9f);
            glVertex3f(2*aspectRatio*i/BUFFER_SIZE-aspectRatio, ud.data[i], 0.f);
        }
        glEnd();

        glBegin(GL_LINE_STRIP);
        PaUtil_ReadRingBuffer(&ud.buffer, &ud.data, BUFFER_SIZE);
        for (int i = 0; i < BUFFER_SIZE; ++i)
        {
            glColor3f(1.0f,0.0f,1.0f);
            double logI = log10(i)*(FFT_SIZE/2+1)/log10(FFT_SIZE/2+1);
            glVertex3f(2*aspectRatio*logI/(FFT_SIZE/2 + 1)-aspectRatio, 2*(dbRange+ud.fft_mag[i])/dbRange-1, 0.f);
        }
        glEnd();
        glBegin(GL_LINES);
        int j = 0;
        int k = 1;
        while(j<(FFT_SIZE/2+1))
        {
            glColor3f(0.5f,0.5f,0.5f);
            double logJ = log10(j)*(FFT_SIZE/2+1)/log10(FFT_SIZE/2+1);
            glVertex3f(2*aspectRatio*logJ/(FFT_SIZE/2 + 1)-aspectRatio, -1, 0.f);
            glVertex3f(2*aspectRatio*logJ/(FFT_SIZE/2 + 1)-aspectRatio, 1, 0.f);
            j+=k;
            if (j==k*10) k*=10;
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
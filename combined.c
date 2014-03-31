#include <GLFW/glfw3.h>
#include <portaudio.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_SECONDS   (2)
#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (64)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#define TABLE_SIZE   (200)
typedef struct
{
    float sine[TABLE_SIZE];
    int left_phase;
    int right_phase;
    char message[20];
}
paTestData;

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

static int onAudioSync (const void* inputBuffer, void* outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void* thunk)
{
    paTestData* data = (paTestData*)thunk;
    float* in = (float*)inputBuffer;
    float* out = (float*)outputBuffer;

    for (int i = 0; i < framesPerBuffer; ++i)
    {
        if (in[i] > 0.1 || in[i] < -0.1) printf("%2f\n", in[i]);
    }
    
    return paContinue;
}

int main (void)
{
    // Create sine wave
    paTestData data;
    for(int i = 0; i < TABLE_SIZE; ++i)
    {
        data.sine[i] = (float)sin(((double)i/(double)TABLE_SIZE) * M_PI * 2.);
    }
    data.left_phase = data.right_phase = 0;

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
    if (out.device == paNoDevice) {
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
                 &data));

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
        glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);

        glBegin(GL_TRIANGLES);
        glColor3f(1.f, 0.f, 0.f);
        glVertex3f(-0.6f, -0.4f, 0.f);
        glColor3f(0.f, 1.f, 0.f);
        glVertex3f(0.6f, -0.4f, 0.f);
        glColor3f(0.f, 0.f, 1.f);
        glVertex3f(0.f, 0.6f, 0.f);
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetTime() > NUM_SECONDS) glfwSetWindowShouldClose(window, GL_TRUE);
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
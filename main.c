#include "gui.h"
#include "live.h"
#include "pitch.h"
#include "midi.h"
#include "glove.h"

// #define GLFW_INCLUDE_GLCOREARB
#include <stdlib.h>
#include <stdio.h>
#include <math.h> //math comes before fftw so that fftw_complex is not overriden

#include <GLFW/glfw3.h>
#include <portaudio.h>
#include <fftw3.h>
#include <portmidi.h>

static void glfwError (int error, const char* description)
{
    fputs(description, stderr);
}

static void pa_check_error (PaError error)
{
    if (error == paNoError) return;

    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", error );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( error ) );
    exit(EXIT_FAILURE);
}

static void on_key_press (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static int on_audio_sync (const void* inputBuffer, void* outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void* userData)
{
    float* in = (float*)inputBuffer;
    //float* out = (float*)outputBuffer;

    for (int i = 0; i < framesPerBuffer; ++i)
    {
        if (in[i] > 1) { live_push_sample(1); printf("%s\n", "clipping high");continue;}
        if (in[i] < -1) { live_push_sample(-1); printf("%s\n", "clipping low");continue;}
        live_push_sample(in[i]);
        // if (in[i] > 0.1) printf("%f\n", in[i]);
    }

    return paContinue;
}

static double x_log_normalize (double unscaled, double logMax)
{
    return log10(unscaled)/logMax;
}

static double db_normalize (double magnitude, double max_magnitude, double dbRange)
{
    double absDb = 10*log10(magnitude/max_magnitude); //between -inf and 0
    return (dbRange+absDb)/dbRange; //between 0 and 1
}

int main (void)
{
    fft_buffer_loc = 0;
    onset_fft_buffer_loc = 0;
    onset_triggered = 0;

    // Initialize Live
    live_init();
    
    // Initialize Midi
    midi_init();
    
    // Initialize RS-232 connection to glove_
    int ser_live;
    int ser_buf[32];
    int ser_buf_loc;
    int midi_channel = 0;
    int angle = 0; //the glove_ is held at an angle, since glove commands come in in pairs, we need to pick just one.
    ser_live = glove_init();

    // Initialize PortAudio
    pa_check_error(Pa_Initialize());

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
    pa_check_error(Pa_OpenStream(&stream,
                                 &in,
                                 &out,
                                 SAMPLE_RATE,
                                 FRAMES_PER_BUFFER,
                                 paClipOff,
                                 on_audio_sync,
                                 NULL));
    pa_check_error(Pa_StartStream(stream));

    // Initialize the GUI
    gui_init();
    
    // Main loop
    int note_on = 0;
    while (!gui_should_exit())
    {
        //SERIAL DATA HANDLING
        if (ser_live)
        {
            ser_buf_loc = glove_poll(ser_buf);
            for (int i=0; i<ser_buf_loc; ++i)
            {
                if (ser_buf[i]<0)
                {
                    if (midi_channel!=ser_buf[i]+128)
                    {
                        midi_channel = ser_buf[i]+128;
                        midi_NOFF(); // clear all notes
                        note_on = 0;
                    }
                }
                else angle = ser_buf[i];
            }
        }

        //MIDI OUT STATEMENTS
        int outputPitch;
        if (onset_average_amplitude > ONSET_THRESHOLD)
        {
            if (!note_on)
            {
                midi_write(Pm_Message(0x90|midi_channel, 54, 100/*(int)average_amplitude*/));
                note_on = 1;
            }
            //0x2000 is 185 hz, 0x0000 is 73.416, 0x3fff is 466.16
            double midiNumber = 12 * log2(dominant_frequency_lp/440) + 69;
            //0x0000 is 38, 0x3fff is 70
            outputPitch = (int)((midiNumber-38)/32*0x3FFF);
            if (outputPitch > 0x3FFF) outputPitch = 0x3FFF;
            if (outputPitch < 0x0000) outputPitch = 0x0000;
            int lsb_7 = outputPitch&0x7F;
            int msb_7 = (outputPitch>>7)&0x7F;
            int outputCentroid = (int)((spectral_centroid-400)/400*127);
            if (outputCentroid > 127) outputCentroid = 127;
            if (outputCentroid < 000) outputCentroid = 000;
            
            if (ser_live) midi_write(Pm_Message(0xB0/*|midi_channel*/, 0, angle));
            midi_write(Pm_Message(0xB0/*|midi_channel*/, 1, outputCentroid));
            midi_write(Pm_Message(0xE0|midi_channel, lsb_7, msb_7));
        }
        else
        {
            if (note_on)
            {
                midi_write(Pm_Message(0x80|midi_channel, 54, 100));
                note_on = 0;
            }
            outputPitch = -INFINITY;
        }
        midi_flush();

        // GUI HANDLING
        gui_redraw();

        //PRINT STATEMENTS
        //        printf("full_spectrum amplitude: %f\n", average_amplitude);
        //        printf("full_spectrum_flatness: %f\n", spectral_flatness);
        //        printf("low_passed_dom_freq : %f\n", dominant_frequency_lp);
        //        printf("first 8 bins: %06.2f %06.2f %06.2f %06.2f %06.2f %06.2f %06.2f %06.2f\n", fft_mag[0], fft_mag[1], fft_mag[2], fft_mag[3], fft_mag[4], fft_mag[5], fft_mag[6], fft_mag[7]);
        //        printf("onset amplitude: %f\n",onset_average_amplitude);
    //            printf("harmonic average vs. lp_pitch: %f %f\n", harmonic_average, dominant_frequency_lp);
        printf("midi channel, angle: %i\t %i\n",midi_channel, angle);
    }
    
    // Shut down the GUI
    gui_cleanup();
    
    // Shut down PortAudio
    pa_check_error(Pa_StopStream(stream));
    pa_check_error(Pa_CloseStream(stream));
    Pa_Terminate();

    // Shut down Midi
    midi_cleanup();
    
    // Exit happily
    exit(EXIT_SUCCESS);
}
#include "backend.h"
#include "dywapitchtrack.h"
#include "gui.h"
#include "pitch.h"
#include "midi.h"
#include "serial.h"
#include "windowing.h"

// #define GLFW_INCLUDE_GLCOREARB
#include <stdlib.h>
#include <stdio.h>
#include <math.h> //math comes before fftw so that fftw_complex is not overriden

#include <GLFW/glfw3.h>
#include <portaudio.h>
#include <fftw3.h>
#include <portmidi.h>

static void pa_check_error (PaError error)
{
    if (error == paNoError) return;

    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", error );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( error ) );
    exit(EXIT_FAILURE);
}

static int on_audio_sync (const void* inputBuffer, void* outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void* userData)
{
    float* in = (float*)inputBuffer;

    for (int i = 0; i < framesPerBuffer; ++i)
    {
        if (in[i] < -1 || in[i] > 1) printf("clipping\n");
        double sample = in[i] > 1 ? 1 : in[i] < -1 ? -1 : in[i];
        if (backend_push_sample(sample)) gui_fft_filled();
    }

    return paContinue;
}

int main (void)
{
    fft_buffer_loc = 0;
    onset_fft_buffer_loc = 0;
    onset_triggered = 0;

    // Initialize Live
    backend_init();
    
    // Initialize Midi
    midi_init();
    
    // Initialize RS-232 connection to glove_
    int ser_live;
    int ser_buf_size = 256;
    char ser_buf[ser_buf_size];
    int midi_channel = 0;
    int angle = 0; //the glove_ is held at an angle, since glove commands come in in pairs, we need to pick just one.
    ser_live = serial_init();

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
    note_on = 0;
    while (!gui_should_exit())
    {
        //SERIAL DATA HANDLING
        if (ser_live)
        {
            size_t bytes_read = serial_poll(ser_buf, ser_buf_size);
            for (int i = 0; i < bytes_read; ++i)
            {
                if (ser_buf[i]<0)
                {
                    if (midi_channel!=ser_buf[i]+128)
                    {
                        if ((ser_buf[i]+128 == 4 )|| (midi_channel == 4)) midi_write(Pm_Message(0xB0|midi_channel, 10, 0)); //toggle solo
                        midi_channel = ser_buf[i]+128;
                        midi_NOFF(); // clear all notes
                        note_on = 0;
                    }
                }
                else angle = ser_buf[i];
            }
        }

        //MIDI OUT STATEMENTS
        int outputPitch = -INFINITY;
        if (onset_average_amplitude>ONSET_THRESHOLD)
        {
            if (!note_on)
            {
                midi_write(Pm_Message(0x90|midi_channel, 54, 100/*(int)average_amplitude*/));
                // printf("midi on\n");
                note_on = 1;
            }
            //0x2000 is 185 hz, 0x0000 is 73.416, 0x3fff is 466.16
            double midiNumber = 12 * log2(dominant_frequency_lp/440) + 69;
            //0x0000 is 38, 0x3fff is 70
            outputPitch = (int)((midiNumber-38)/32*0x3FFF);
            if (outputPitch > 0x3FFF) outputPitch = 0x3FFF;
            if (outputPitch < 0x0000) outputPitch = 0x0000;

            int outputCentroid = (int)((spectral_centroid-500)/300*127);
            if (outputCentroid > 127) outputCentroid = 127;
            if (outputCentroid < 000) outputCentroid = 000;
            if (prev_spectral_centroid != -INFINITY){
                if (abs(outputCentroid-prev_spectral_centroid)>127){
                    outputCentroid = prev_spectral_centroid;
                }
                else prev_spectral_centroid = outputCentroid;
            }
            else prev_spectral_centroid = outputCentroid;
            
            if (prev_output_pitch != -INFINITY){
                if ((outputPitch == 0) || (outputPitch == 0x3FFF)){
                    outputPitch = prev_output_pitch;
                }
                else prev_output_pitch = outputPitch;
            }
            else prev_output_pitch = outputPitch;
            int lsb_7 = outputPitch&0x7F;
            int msb_7 = (outputPitch>>7)&0x7F;

            
            if (ser_live) midi_write(Pm_Message(0xB0/*|midi_channel*/, 0, angle));
            midi_write(Pm_Message(0xB0/*|midi_channel*/, 1, outputCentroid));
//            printf("centroid out: %03u pitch out: %04u\n", outputCentroid, outputPitch);
            midi_write(Pm_Message(0xE0|midi_channel, lsb_7, msb_7));
        }
        else if (onset_average_amplitude<OFFSET_THRESHOLD)
        {
            if (note_on)
            {
                midi_write(Pm_Message(0x80|midi_channel, 54, 100));
                // printf("midi off\n");
                note_on = 0;
                dywapitch_inittracking(&pitch_tracker);
                prev_spectral_centroid = -INFINITY;
                prev_output_pitch = -INFINITY;
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
        //        printf("harmonic average vs. lp_pitch: %f %f\n", harmonic_average, dominant_frequency_lp);
        // printf("midi channel, angle: %i\t %i\n",midi_channel, angle);
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
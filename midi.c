#include "midi.h"

#include <stdlib.h>

#include <portmidi.h>

#define BUFFER_CAPACITY 32

static PmStream* stream;
static PmEvent   buffer[BUFFER_CAPACITY];
static size_t    buffer_size;

void midi_init ()
{
    Pm_OpenOutput(&stream,
                  Pm_GetDefaultOutputDeviceID(), // Output device
                  NULL,                          //
                  0,                             // Unused buffer size
                  NULL,                          //
                  NULL,                          //
                  0);                            // Latency
}

void midi_cleanup ()
{
    Pm_Close(stream);
    Pm_Terminate();
    for (size_t i = 0; i < BUFFER_CAPACITY; ++i) buffer[i].timestamp = 0;
}

int midi_write (int message)
{
    if (buffer_size >= BUFFER_CAPACITY) return 1;
    buffer[buffer_size].message = message;
    ++buffer_size;
    return 0;
}

void midi_NOFF ()
{
    midi_write(Pm_Message(0x80, 54, 0));
    midi_write(Pm_Message(0x81, 54, 0));
    midi_write(Pm_Message(0x82, 54, 0));
    midi_write(Pm_Message(0x83, 54, 0));
}
void midi_flush ()
{
    Pm_Write(stream, buffer, (int)buffer_size);
    buffer_size = 0;
}
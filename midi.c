#include <portmidi.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
    PmStream* midi;

    Pm_OpenOutput(&midi, 
                  Pm_GetDefaultOutputDeviceID(), // Output device
                  NULL,                          //
                  0,                             // Output buffer size
                  NULL,                          //
                  NULL,                          //
                  0);                            // Latency

    Pm_WriteShort(midi, 0, Pm_Message(0x90, 60, 100));
    
    Pm_Close(midi);
    Pm_Terminate();
}
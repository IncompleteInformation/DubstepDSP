#include "midi.h"

int main (void)
{
    midi_init();

    // Play a chord
    midi_write(Pm_Message(0x90, 60, 100));
    midi_write(Pm_Message(0x90, 64, 100));
    midi_write(Pm_Message(0x90, 67, 100));
    midi_flush();

    midi_cleanup();
    return 0;
}
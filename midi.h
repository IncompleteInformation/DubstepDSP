#include <portmidi.h>

// Initialize PortMidi
void midi_init ();

// Release PortMidi resources
void midi_cleanup ();

// Append a message to the MIDI buffer
// Return 0 on success, 1 on error.
int midi_write (long message);

// Send the MIDI buffer
void midi_flush ();
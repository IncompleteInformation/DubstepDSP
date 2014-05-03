#include <stdbool.h>

// Initialize the GUI
void gui_init ();

// Update the GUI
void gui_redraw ();

// Called when FFT becomes full
void gui_fft_filled ();

// Return true if the user has requested that the GUI be closed
bool gui_should_exit ();

// Destroy the GUI
void gui_cleanup ();
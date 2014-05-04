//sets up midi connection
int serial_init();

// Read data from serial device
// Return the number of bytes read, or -1 if none.
//   out:      the buffer to be populated with data
//   out_size: the size of the output buffer
size_t serial_poll (char* out, size_t out_size);

// Shut down serial connection
void serial_close ();
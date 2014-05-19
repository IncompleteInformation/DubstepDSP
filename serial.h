//sets up midi connection
int serial_init();

int serial_out_init();

// Read data from serial device
// Return the number of bytes read, or -1 if none.
//   out:      the buffer to be populated with data
//   out_size: the size of the output buffer
size_t serial_poll (char* out, size_t out_size);

// Shut down serial connection
void serial_close ();

// Write data to serial device
// Return the number of bytes written, or -1 if failed.
//   out:      the data to be written
int serial_out_write(char out);

void serial_out_clear();
#include <unistd.h>  /* UNIX standard function definitions */
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/tty.linvor-DevB"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

//sets up midi connection
int glove_init();

// Read data from glove device
// Return the number of bytes read.
//   output: the buffer to be populated with data (TODO: what length???)
int glove_poll (int* output);

// Shut down glove connection
void glove_close ();

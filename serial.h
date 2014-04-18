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
int serial_setup();

//polls for data
//output: list to be populated with stuff
//returns length of output
int serial_poll(int *output);

//shut down connection
void serial_close();

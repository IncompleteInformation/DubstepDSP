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

//volatile int STOP=FALSE;

int fd;
struct termios oldtio,newtio;
char buf[255];


int serial_init()
{
    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(MODEMDEVICE); return 0; }
    
    tcgetattr(fd,&oldtio); /* save current port settings */
    
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    
    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    
    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */
    
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
    return 1;
}

size_t serial_poll (char* out, size_t out_size)
{
    return read(fd,out,out_size);   /*TODO: figure this out: returns after 5 chars have been input */
}

void serial_close ()
{
    tcsetattr(fd,TCSANOW,&oldtio);
}

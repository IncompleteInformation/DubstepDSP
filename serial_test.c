#include <stdio.h>
#include "serial.h"

int main (void)
{
    int serfail;
    serfail = serial_setup();
    if (!serfail) return 0;
    
    int len;
    int out[32];
    for (int i = 0; i<10000000; ++i)
    {
        len = serial_poll(out);
        for (int j = 0; j<len; ++j)
        {
            printf("%i:", out[j]);
        }
        printf("\n");
    }
    serial_close();
    return 1;
}
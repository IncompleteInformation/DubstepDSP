#include <stdio.h>
#include <unistd.h>
#include "serial.h"

// int main (void) //input test
// {
//     int ser_live;
//     ser_live = serial_init();
//     //if (!serfail) return 0;
    
//     int len;
//     int out[32];
//     if (ser_live)
//     {
//         for (int i = 0; i<10000000; ++i)
//         {
//             len = serial_poll(out);
//             for (int j = 0; j<len; ++j)
//             {
//                 printf("%i:", out[j]);
//             }
//             printf("\n");
//         }
        
//         serial_close();
//     }
//     printf("casual continue\n");
//     return 1;
// }

int main (void) //output test
{
    printf("echo\n");
    int out_live = serial_out_init();
    printf("echo2\n");
    if (out_live) 
    {
        for (int i = 0; i < 255; i++)
        {
            int err = serial_out_write((char)i);
            usleep(50000);
        }
    }
    else {printf("write failure\n"); return -1;}
    serial_close();
    return 1;
}
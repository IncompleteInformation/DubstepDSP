#include <stdio.h>
#include "glove.h"

int main (void)
{
    int ser_live;
    ser_live = glove_init();
    //if (!serfail) return 0;
    
    int len;
    int out[32];
    if (ser_live)
    {
        for (int i = 0; i<10000000; ++i)
        {
            len = glove_poll(out);
            for (int j = 0; j<len; ++j)
            {
                printf("%i:", out[j]);
            }
            printf("\n");
        }
        
        glove_close();
    }
    printf("casual continue\n");
    return 1;
}
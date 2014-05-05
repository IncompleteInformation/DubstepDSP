#include "backend.h"

#include <sndfile.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char* after (char* str, char c)
{
    if (!str) return NULL;
    size_t strl = strlen(str);
    for (int i = strl-2; i > 0; --i)
    {
        if (str[i] == c) return &str[i+1];
    }
    return str;
}

bool test_file (char* file)
{
    SF_INFO info;
    SNDFILE* f = sf_open(file, SFM_READ, &info);
    if (f == NULL)
    {
        fprintf(stderr, "FAILED: %s\n    File not found.\n", file);
        return false;
    }
    double freq = atof(after(file, '/'));
    double* sample = malloc(sizeof(double)*info.frames);
    sf_read_double(f, sample, info.frames);
    
    for (int i = 0; i < info.frames; ++i)
    {
        if (backend_push_sample(sample[i]))
        {
            double t = 1.0 * i / info.samplerate;
            printf("%s, %f, %f, %f\n", file, t, freq, dominant_frequency_lp);
        }
    }

    sf_close(f);
    return true;
}

int main (int argc, char** argv)
{
    backend_init();

    printf("File, Time, Freq, dominant_frequency_lp\n");
    for (int i = 1; i < argc; ++i)
    {
        test_file(argv[i]);
    }

    return 0;
}
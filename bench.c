#include "backend.h"
#include "dywapitchtrack.h"
#include "tinydir.h"

#include <sndfile.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static dywapitchtracker pitch_tracker;

bool ends_with (char* str, char* suffix)
{
    if (!str || !suffix) return false;
    size_t strl = strlen(str);
    size_t sufl = strlen(suffix);
    if (sufl > strl) return false;
    return strncmp(str + strl - sufl, suffix, sufl) == 0;
}

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

void test_file (char* file)
{
    if (!ends_with(file, ".wav")) return;

    SF_INFO info;
    SNDFILE* f = sf_open(file, SFM_READ, &info);
    if (f == NULL)
    {
        fprintf(stderr, "Failed to open file: %s\n", file);
        return;
    }
    double freq = atof(after(file, '/'));
    double* sample = malloc(sizeof(double) * info.frames);
    sf_read_double(f, sample, info.frames);
    
    for (int i = 0; i+1024 <= info.frames; i += 1024)
    {
        double t = 1.0 * i / info.samplerate;
        double pitch = dywapitch_computepitch(&pitch_tracker, sample, i, 1024);
        printf("%s, %f, %f, %f\n", file, t, freq, pitch);
        // if (backend_push_sample(sample[i]))
        // {
        //     double t = 1.0 * i / info.samplerate;
        //     printf("%s, %f, %f, %f\n", file, t, freq, dominant_frequency_lp);
        // }
    }

    sf_close(f);
}

void test_dir (char* path)
{
    tinydir_dir dir;
    tinydir_open(&dir, path);

    while (dir.has_next)
    {
        tinydir_file file;
        tinydir_readfile(&dir, &file);
        if (file.name[0] != '.')
        {
            char file_path[1024];
            strcpy(file_path, path);
            strcat(file_path, "/");
            strcat(file_path, file.name);

            if (file.is_dir) test_dir(file_path);
            else             test_file(file_path);
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);
}

int main (int argc, char** argv)
{
    // backend_init();
    dywapitch_inittracking(&pitch_tracker);

    printf("File, Time, Pitch, PitchGuess\n");

    char path[1024];
    getcwd(path, 1024);
    strcat(path, "/samples");
    test_dir(path);
}
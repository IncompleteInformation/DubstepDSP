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

#include <GLFW/glfw3.h>

#define HISTOGRAM_BINS 10
#define HISTOGRAM_RESOLUTION 1.0
#define NUM_FILES (33*4)

static GLFWwindow* pitchAccuracyWindow;
static double all_histograms[NUM_FILES][HISTOGRAM_BINS*2+1];
static int counter = 0;

static int    width, height, numFiles;
static float  aspectRatio;

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

static void on_key_press (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void on_glfw_error (int error, const char* description)
{
    fputs(description, stderr);
}

static void switch_focus(GLFWwindow* focus)
{
    glfwPollEvents();

    glfwMakeContextCurrent(focus);
    glfwSetKeyCallback(focus, on_key_press);
    
    glfwGetFramebufferSize(focus, &width, &height);
    aspectRatio = width / (float) height;
    
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glClearColor(1, 1, 1, 1);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-aspectRatio, aspectRatio, -1.f, 1.f, 1.f, -1.f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void gui_init ()
{
    glfwSetErrorCallback(on_glfw_error);
    if (!glfwInit()) exit(EXIT_FAILURE);
    pitchAccuracyWindow = glfwCreateWindow(640, 480, "Pitch Accuracy Analysis", NULL, NULL);
    if (!pitchAccuracyWindow)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
}

static void gui_cleanup ()
{
    glfwDestroyWindow(pitchAccuracyWindow);
    glfwTerminate();
}

static bool gui_should_exit ()
{
    return glfwWindowShouldClose(pitchAccuracyWindow);
}

static double* histogram(size_t len_data, double* data, double targetFreq, size_t num_bins)
{
    double* out = malloc(sizeof(double) * (num_bins*2+1));
    double tolerance =  1.0;
    double offset;
    int histbin   = -1;
    for (int i = 0; i < num_bins*2+1; i++) out[i] = 0;
    for (int i = 0; i < len_data; i++)
    {
        offset = data[i] - targetFreq;
        if      (offset == 0) histbin = num_bins;
        else if (offset <  0) 
        {
            offset *= -1;
            histbin = num_bins - (log2(offset) * tolerance);
            if (histbin < 0       ) histbin = 0;
            if (histbin > num_bins-1) histbin = num_bins - 1;
        }
        else
        {
            histbin = num_bins + (log2(offset) * tolerance);
            if (histbin < num_bins    ) histbin =     num_bins;
            if (histbin > 2 * num_bins) histbin = 2 * num_bins;           
        }
        out[(int)histbin] += 1.0 / len_data;
    }
    return out;
}

static void graph_histograms()
{
    for (int i = 0; i<NUM_FILES; ++i)
    {
        double curYBot = 2 * (double) i   /NUM_FILES - 1;
        double curYTop = 2 * (double)(i+1)/NUM_FILES - 1;
        glBegin(GL_QUAD_STRIP);
        glVertex3f(-aspectRatio, curYBot, 0);
        glVertex3f(-aspectRatio, curYTop, 0);
        for (int j = 0; j<HISTOGRAM_BINS*2+2; ++j)
        {
            double curXRight = aspectRatio * (2 * ((double)j / (HISTOGRAM_BINS*2+1)) - 1);
            
            double scaledMag = all_histograms[i][j];
            // printf("%f\n", scaledMag);
            glColor3f(scaledMag, scaledMag, scaledMag);
            glVertex3f(curXRight, curYTop, 0);
            glVertex3f(curXRight, curYBot, 0);
        }
        glEnd();
    }
}

static void gui_redraw ()
{
    switch_focus(pitchAccuracyWindow);
    graph_histograms();
    glfwSwapBuffers(pitchAccuracyWindow);
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
    double* values = malloc(sizeof(double) * info.frames/1024);

    sf_read_double(f, sample, info.frames);
    
    for (int i = 0; i+1024 <= info.frames; i += 1024)
    {
        double t = 1.0 * i / info.samplerate;

        for (int j = 0; j < 1024; ++j)
        {
            backend_push_sample(sample[i+j]);
        }
        double pitch = spectral_centroid;
        values[i/1024] = pitch;
        printf("%f\t%f\n", freq, pitch);
    } 
    double* out = malloc(sizeof(double) * (HISTOGRAM_BINS*2+1));
    out = histogram(info.frames/1024, values, 0, HISTOGRAM_BINS);
    for (int i = 0; i < (HISTOGRAM_BINS*2+1); i++) all_histograms[counter][i] = out[i];
    counter++;
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
    backend_init();
    gui_init();

    // printf("File, Time, Pitch, PitchGuess\n");

    char path[1024];
    getcwd(path, 1024);
    strcat(path, "/samples/3_aaa");
    test_dir(path);
    while (!gui_should_exit())
    {
        gui_redraw();
    }
    gui_cleanup();
}
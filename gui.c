#include "backend.h"
#include "gui.h"
#include "windowing.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

// #define GLFW_INCLUDE_GLCOREARB // Enable OpenGL 3
#include <GLFW/glfw3.h>
#include <GL/freeglut.h>

#define PITCHTRACKERLISTSIZE 256
#define SPECTROGRAM_LENGTH   100

static GLFWwindow* trackerWindow;
static GLFWwindow* mainWindow;

static double dbRange;
static int    width, height;
static float  aspectRatio;
static float  pitchTrackerList[PITCHTRACKERLISTSIZE];
static pthread_mutex_t spectrogram_lock;

static int spectrogram_buffer_loc;
static double spectrogram_buffer[SPECTROGRAM_LENGTH][FFT_SIZE/2+1];
static double pitch_lp_buffer[SPECTROGRAM_LENGTH];

static void on_key_press (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_0 && action == GLFW_PRESS) window_function = RECTANGLE;
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) window_function = WELCH;
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) window_function = HANNING;
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) window_function = HAMMING;
    if (key == GLFW_KEY_4 && action == GLFW_PRESS) window_function = BLACKMAN;
    if (key == GLFW_KEY_5 && action == GLFW_PRESS) window_function = NUTTAL;
}

static double x_log_normalize (double unscaled, double logMax)
{
    return log10(unscaled)/logMax;
}

static double db_normalize (double magnitude, double max_magnitude, double dbRange)
{
    double absDb = 10*log10(magnitude/max_magnitude); //between -inf and 0
    return (dbRange+absDb)/dbRange; //between 0 and 1
}

static void on_glfw_error (int error, const char* description)
{
    fputs(description, stderr);
}

static void graph_log_lines ()
{
    //log lines
    glBegin(GL_LINES);
    int j = 0;
    int k = 10;
    double logLogLinesX = log10(SAMPLE_RATE/2);
    while(j<(SAMPLE_RATE/2))
    {
        glColor3f(0.6f,0.4f,0.1f);
        double logJ = x_log_normalize((double)j, logLogLinesX);
        glVertex3f(aspectRatio*(2*logJ-1), -1, 0.f);
        glVertex3f(aspectRatio*(2*logJ-1), 1, 0.f);
        j+=k;
        if (j==k*10) k*=10;
    }
    glEnd();
}

static void graph_fft_mag (int dbRange)
{
    //fft_mag graph (db, log)
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f,0.0f,1.0f);
    double logMax = log10(SAMPLE_RATE/2);
    for (int i = 0; i < FFT_SIZE/2+1; ++i)
    {
        double logI = x_log_normalize(i*BIN_SIZE, logMax);
        double scaledMag = db_normalize(fft_mag[i], 1, dbRange); //max amplitude is FFT_SIZE/2)^2
        glVertex3f(2*aspectRatio*logI-aspectRatio, 2*scaledMag-1, 0.f);
    }
    glEnd();
}

static void graph_spectral_centroid ()
{
    //specral centroid marker
    glBegin(GL_LINES);
    glColor3f(1.f, 0.f, 0.f);
    double logCentroid = log10(spectral_centroid)*(SAMPLE_RATE/2)/log10(SAMPLE_RATE/2+1);
    glVertex3f(2*aspectRatio*logCentroid/(SAMPLE_RATE/2)-aspectRatio, -1, 0.f);
    glVertex3f(2*aspectRatio*logCentroid/(SAMPLE_RATE/2)-aspectRatio, 1, 0.f);
    glEnd();
}

static void graph_dominant_pitch_lp ()
{
    //dominant pitch line (lowpassed)
    glBegin(GL_LINES);
    glColor3f(0.f, 1.f, 1.f);
    double domlogMax = log10(SAMPLE_RATE/2);
    double logNormDomFreq_lp = x_log_normalize(dominant_frequency_lp, domlogMax);
    glVertex3f(aspectRatio*(2*logNormDomFreq_lp-1), -1, 0.f);
    glVertex3f(aspectRatio*(2*logNormDomFreq_lp-1), 1, 0.f);
    glEnd();
}

static void graph_spectrogram (int dbRange)
{
    double logMax = log10(SAMPLE_RATE/2);
    for (int i = 0; i<SPECTROGRAM_LENGTH; ++i)
    {
        double curXLeft  = aspectRatio * (2 * (double)i/SPECTROGRAM_LENGTH - 1);
        double curXRight = aspectRatio * (2 * (double)(i+1)/SPECTROGRAM_LENGTH -1);
        glBegin(GL_QUAD_STRIP);
        glVertex3f(curXLeft , -1, 0);
        glVertex3f(curXRight, -1, 0);
        for (int j = 1; j<FFT_SIZE/2+1; ++j)
        {
            //draw QUAD for each bin?
            double logJ  = x_log_normalize(j*BIN_SIZE, logMax);
            double curYTop = 2*logJ-1;
            double scaledMag = spectrogram_buffer[(i+spectrogram_buffer_loc)%SPECTROGRAM_LENGTH][j];
            glColor3f(scaledMag, scaledMag, scaledMag);
            glVertex3f(curXLeft, curYTop, 0.f);
            glVertex3f(curXRight, curYTop, 0.f);
        }
        glEnd();
    }
}

static void graph_spectrogram_3d_parallel (int dbRange)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(-1.0,0.0,-8.0);
    glRotatef(45.0,1.0,0.0,0.0);
    glScalef(4.0,2.0,1.0);
    double logMax = log10(SAMPLE_RATE/2);
    for (int i = 0; i<SPECTROGRAM_LENGTH; ++i)
    {
        glBegin(GL_LINE_STRIP);
        glColor3f(1-(.5+.5*((double)i / SPECTROGRAM_LENGTH)),1-((double)i / SPECTROGRAM_LENGTH),1-((double)i / SPECTROGRAM_LENGTH));
        for (int j = 0; j < FFT_SIZE/2+1; ++j)
        {
            double logJ = x_log_normalize(j*BIN_SIZE, logMax);
            double scaledMag = spectrogram_buffer[(i+spectrogram_buffer_loc)%SPECTROGRAM_LENGTH][j];
            glVertex3f(2*logJ-1, 2*scaledMag-1, 8*(double)i / SPECTROGRAM_LENGTH - 4);
        }
        glEnd();
    }
}

static void graph_spectrogram_3d_normal (int dbRange)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(-1.0,0.0,-8.0);
    glRotatef(45.0,1.0,0.0,0.0);
    glScalef(4.0,2.0,1.0);
    double logMax = log10(SAMPLE_RATE/2);
    for (int i = 0; i<FFT_SIZE/2+1; ++i)
    {
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j < SPECTROGRAM_LENGTH; ++j)
        {
            glColor3f(1-(.5+.5*((double)j / SPECTROGRAM_LENGTH)),1-((double)j / SPECTROGRAM_LENGTH),1-((double)j / SPECTROGRAM_LENGTH));
            double logI = x_log_normalize(i*BIN_SIZE, logMax);
            double scaledMag = spectrogram_buffer[(j+spectrogram_buffer_loc)%SPECTROGRAM_LENGTH][i];
            glVertex3f(2*logI-1, 2*scaledMag-1, 8*(double)j / SPECTROGRAM_LENGTH - 4);
        }
        glEnd();
    }
}

static void rainbow_calc(double s, GLdouble * ret)
{
  int num_stages = 6;
  float a,b,c,d,e,f,rf,gf,bf;
  a = 1.0/num_stages; b = 2*a; c = 3*a; d = 4*a; e = 5*a; f = 6*a;
  
  if      ( s < a ) { rf = ( s - 0 )     * num_stages; rf = rf; gf =  0; bf =  0; }
  else if ( s < b ) { bf = ( s - a )     * num_stages; rf =  1; gf =  0; bf = bf; }
  else if ( s < c ) { rf = 1 - ( s - b ) * num_stages; rf = rf; gf =  0; bf =  1; }
  else if ( s < d ) { gf = ( s - c )     * num_stages; rf =  0; gf = gf; bf =  1; }
  else if ( s < e ) { bf = 1 - ( s - d ) * num_stages; rf =  0; gf =  1; bf = bf; }
  else if ( s < f ) { rf = ( s - e )     * num_stages; bf = ( s - e ) * num_stages; rf = rf; gf =  1; bf = bf; }

  ret[0] = 1-rf; ret[1] = 1-gf; ret[2] = 1-bf;
}
static void graph_spectrogram_3d_poly (int dbRange)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(-aspectRatio,0.0,-8.0);
    glRotatef(40.0,1.0,0.0,0.0);
    // glRotatef(20.0,0.0,-4.0,0.0);
    glScalef(6.0,2.0,1.0);

    double logMax = log10(SAMPLE_RATE/2);
    for (int i = 0; i<SPECTROGRAM_LENGTH-1; ++i)
    {
        GLdouble cur_color[3] = {0,0,0};
        // rainbow_calc((double)i / SPECTROGRAM_LENGTH, cur_color);
        // glColor3dv(cur_color);
        glColor3f(1.25-(.5+.5*((double)i / SPECTROGRAM_LENGTH)),1.25-((double)i / SPECTROGRAM_LENGTH),1.25-((double)i / SPECTROGRAM_LENGTH));
        for (int j = 0; j < FFT_SIZE/2; ++j)
        {
            glBegin(GL_QUADS);
            double logJ0 = x_log_normalize(j*BIN_SIZE, logMax);
            double logJ1 = x_log_normalize((j+1)*BIN_SIZE, logMax);
            double scaledMag0 = spectrogram_buffer[(i+spectrogram_buffer_loc  )%SPECTROGRAM_LENGTH][j  ];
            double scaledMag1 = spectrogram_buffer[(i+spectrogram_buffer_loc+1)%SPECTROGRAM_LENGTH][j  ];
            double scaledMag2 = spectrogram_buffer[(i+spectrogram_buffer_loc+1)%SPECTROGRAM_LENGTH][j+1];
            double scaledMag3 = spectrogram_buffer[(i+spectrogram_buffer_loc  )%SPECTROGRAM_LENGTH][j+1];
            glVertex3f(2*logJ0-1, 2*scaledMag0-1, 8*(double)i     / SPECTROGRAM_LENGTH - 4);
            glVertex3f(2*logJ0-1, 2*scaledMag1-1, 8*(double)(i+1) / SPECTROGRAM_LENGTH - 4);
            glVertex3f(2*logJ1-1, 2*scaledMag2-1, 8*(double)(i+1) / SPECTROGRAM_LENGTH - 4);
            glVertex3f(2*logJ1-1, 2*scaledMag3-1, 8*(double)i     / SPECTROGRAM_LENGTH - 4);
            glEnd();
        }
        glBegin(GL_QUADS);
        glColor3f(1.25-(.5+.5*((double)i / SPECTROGRAM_LENGTH)),1.25-(.5+.5*(double)i / SPECTROGRAM_LENGTH),1.25-((double)i / SPECTROGRAM_LENGTH));
        double curFreq  = pitch_lp_buffer[(i+spectrogram_buffer_loc  )%SPECTROGRAM_LENGTH];
        double prevFreq = pitch_lp_buffer[(i+spectrogram_buffer_loc+1)%SPECTROGRAM_LENGTH];
        double logCurFreq  = x_log_normalize(curFreq , logMax);
        double logPrevFreq = x_log_normalize(prevFreq, logMax);
        glVertex3f(2 * logCurFreq  -1, 2, 8*(double)i     / SPECTROGRAM_LENGTH - 4);
        glVertex3f(2 * logPrevFreq -1, 2, 8*(double)(i+1) / SPECTROGRAM_LENGTH - 4);
        glVertex3f(2 * logPrevFreq -1, 0, 8*(double)(i+1) / SPECTROGRAM_LENGTH - 4);
        glVertex3f(2 * logCurFreq  -1, 0, 8*(double)i     / SPECTROGRAM_LENGTH - 4);
        glEnd();

        glFlush();
    }
}
static void draw_square_cw(float l)
{
    glBegin(GL_QUADS);
    glVertex3f(0, 0, 0);
    glVertex3f(0, l, 0);
    glVertex3f(l, l, 0);
    glVertex3f(l, 0, 0);
    glEnd();
}
static void draw_square_ccw(float l)
{
    glBegin(GL_QUADS);
    glVertex3f(0, 0, 0);
    glVertex3f(l, 0, 0);
    glVertex3f(l, l, 0);
    glVertex3f(0, l, 0);
    glEnd();
}
static void draw_cube(float l)
{
    glColor3f(0,0,0);
    //front
    draw_square_cw(l);
    //bottom
    glLoadIdentity();
    glRotatef(90, -1, 0, 0);
    draw_square_cw(l);
    //left
    glLoadIdentity();
    glRotatef(90, 0,  1, 0);
    draw_square_cw(l);
    //back
    glLoadIdentity();
    glTranslatef(0, 0, -l);
    draw_square_ccw(l);
    //top
    glLoadIdentity();
    glRotatef(90, -1, 0, 0);
    glTranslatef(0, 1, 0);
    draw_square_ccw(l);
    //right
    glLoadIdentity();
    glRotatef(90, 0, 1, 0);
    glTranslatef(1, 0, 0);
    draw_square_ccw(l);
}
static void graph_x_log_lines()
{
    glBegin(GL_LINES);
    for (int i = 0; i<36; ++i)
    {
        glColor3f(0.9f,0.7f,0.5f);
        glVertex3f(aspectRatio*-1, 2*(i/36.f)-1, 0.f);
        glVertex3f(aspectRatio, 2*(i/36.f)-1, 0.f);
    }
    glEnd();
}

static void graph_midi_notes()
{
    for (int i = 1; i < PITCHTRACKERLISTSIZE; ++i)
    {
        pitchTrackerList[i] = pitchTrackerList[i+1];
    }

    double midiNumber = 12 * log2(dominant_frequency_lp/440) + 69;
    int outputPitch = (int)((midiNumber-38)/32*0x3FFF);
    if (onset_average_amplitude < ONSET_THRESHOLD) outputPitch = -INFINITY;
    pitchTrackerList[PITCHTRACKERLISTSIZE-1] = (float)outputPitch/0x3FFF;
    
    glBegin(GL_LINES);
    glColor3f(0.f, 0.f, 0.1f);

    for (int i = 0; i < PITCHTRACKERLISTSIZE; ++i)
    {
        float xPos1 = aspectRatio*((2*i/(float)PITCHTRACKERLISTSIZE)-1);
        float xPos2 = aspectRatio*((2*(i+1)/(float)PITCHTRACKERLISTSIZE)-1);
        float yPos = 2*pitchTrackerList[i]-1;
        glVertex3f(xPos1, yPos, 0.f);
        glVertex3f(xPos2, yPos, 0.f);
    }
    glEnd();
}

static void graph_NON_NOFFs()
{
    glBegin(GL_LINES);
    for (int i = 0; i < PITCHTRACKERLISTSIZE; ++i)
    {
        if (i!=0)
        {
            if ((pitchTrackerList[i-1] < 0) & (pitchTrackerList[i]>=0))
            {
                float xPos = aspectRatio*((2*i/(float)PITCHTRACKERLISTSIZE)-1);
                glColor3f(0.0f, 0.3f, 0.0f);
                glVertex3f(xPos, -1, 0.f);
                glVertex3f(xPos, 1, 0.f);
            }
        }
        if (i!=PITCHTRACKERLISTSIZE)
        {
            if ((pitchTrackerList[i+1] < 0) & (pitchTrackerList[i]>=0))
            {
                float xPos = aspectRatio*((2*i/(float)PITCHTRACKERLISTSIZE)-1);
                glColor3f(0.3f, 0.0f, 0.0f);
                glVertex3f(xPos, -1, 0.f);
                glVertex3f(xPos, 1, 0.f);
            }
        }

    }
    glEnd();
}

static void switch_focus(GLFWwindow* focus)
{
    glfwPollEvents();

    glfwMakeContextCurrent(focus);
    glfwSetKeyCallback(focus, on_key_press);
    
    glfwGetFramebufferSize(focus, &width, &height);
    aspectRatio = width / (float) height;
    
    //glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1, 1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glTranslatef(0, 0, 0);
    //glOrtho(-aspectRatio, aspectRatio, -1.f, 1.f, 1.f, -1.f);
    //glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0,0,width, height);
}

void gui_init ()
{
    // Initialize OpenGL window
    dbRange = 96;
    glfwSetErrorCallback(on_glfw_error);
    if (!glfwInit()) exit(EXIT_FAILURE);
    trackerWindow = glfwCreateWindow(640, 480, "Pitch Tracking", NULL, NULL);
    mainWindow = glfwCreateWindow(640, 480, "Main Analysis", NULL, NULL);
    if (!mainWindow || !trackerWindow)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    pthread_mutex_init(&spectrogram_lock, NULL);
}

void gui_cleanup ()
{
    // Shut down GLFW
    glfwDestroyWindow(mainWindow);
    glfwDestroyWindow(trackerWindow);
    glfwTerminate();
}

bool gui_should_exit ()
{
    return glfwWindowShouldClose(mainWindow) || glfwWindowShouldClose(trackerWindow);
}

void gui_fft_filled ()
{
    pthread_mutex_lock(&spectrogram_lock);
    for (int i = 0; i < FFT_SIZE/2+1; ++i)
    {
        spectrogram_buffer[spectrogram_buffer_loc][i] = db_normalize(fft_mag[i], 1, 96);
        pitch_lp_buffer[spectrogram_buffer_loc] = dominant_frequency_lp;
    }
    spectrogram_buffer_loc = (spectrogram_buffer_loc+1)%SPECTROGRAM_LENGTH;
    pthread_mutex_unlock(&spectrogram_lock);
}

void gui_redraw ()
{
    switch_focus(mainWindow);
 
    //lock   
    // graph_log_lines();
    // graph_fft_mag(dbRange);
    // graph_spectral_centroid();
    // graph_dominant_pitch_lp();
    pthread_mutex_lock(&spectrogram_lock);
    // graph_spectrogram_3d_poly(dbRange);
    pthread_mutex_unlock(&spectrogram_lock);
    draw_cube(1);

    glfwSwapBuffers(mainWindow);

    //PITCH TRACKING WINDOW
    switch_focus(trackerWindow);
    
    graph_x_log_lines();
    graph_midi_notes();
    graph_NON_NOFFs();
    
    glfwSwapBuffers(trackerWindow);
}
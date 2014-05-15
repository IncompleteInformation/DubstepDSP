#include "filter.h"

#include <stdio.h>

int main ()
{
    size_t size = 1024;
    double sample[size], output[size];
    for (int i = 0; i < size; ++i) sample[i] = i%128;
    // for (int i = 0; i < size; ++i) sample[i] += i%32;
    for (int i = 0; i < size; ++i) output[i] = sample[i];
    // band_pass(sample, output, size, 44100, 100, 1000);
    band_pass(sample, output, size, 44100, 0, 44100);
    for (int i = 0; i < size; ++i) printf("%f, %f\n", sample[i], output[i]);
}
#pragma once

#include "MPFR_Complex_Math.h"


typedef struct 
{
    int escape_level;
    mpfr_t smooth_iter;  
} pixel_prop;

typedef struct 
{
    unsigned int height;
    unsigned int width;
    char* x_max;
    char* x_min;
    char* y_max;
    char* y_min;

}margins_val;

typedef struct 
{
    mpfr_t x_max,
    x_min,
    y_max,
    y_min,
    dx,
    dy,
    x,
    y;
}margins_val_mpfr;

status mandelbrot_comp(pixel_prop *map, const margins_val *margins, int max_level);

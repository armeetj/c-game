#ifndef __CUSTOM_UTILS_H__
#define __CUSTOM_UTILS_H__
#include "color.h"
#include "list.h"
#include "body.h"
#include "vector.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


double rand_range(double low, double high);

color_t rgba(float r, float g, float b, float a);

color_t rand_color(double alpha);

color_t rainbow_color(double n, double m);

list_t *make_circle(size_t num_points, size_t length, vector_t center);

list_t *make_rectangle(double width, double height, vector_t center);

#endif // #ifndef __CUSTOM_UTILS_H__
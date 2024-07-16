#ifndef __COLOR_H__
#define __COLOR_H__

#include <stdbool.h>
#include <stdio.h>

/**
 * A color to display on the screen.
 * The color is represented by its red, green, and blue components.
 * Each component must be between 0 (black) and 1 (white).
 */
typedef struct {
  float r;
  float g;
  float b;
} rgb_color_t;

/**
 * A color to display on the screen.
 * The color is represented by its red, green, and blue components.
 * Each component must be between 0 (black) and 1 (white).
 * The color also has an opacity component a.
 * This component must be between 0 (translucent) and 1 (opaque).
 */
typedef struct {
  float r;
  float g;
  float b;
  float a;
} color_t;

/**
 * @param color to be inverted
 * @returns the inverted color
 */
color_t color_inverted(color_t color);

/**
 * Checks if rgba values of 2 colors are same
 * 
 * @param c1 color 1
 * @param c2 color 2
 * @return true 
 * @return false 
 */
bool color_equals(color_t c1, color_t c2);

#endif // #ifndef __COLOR_H__
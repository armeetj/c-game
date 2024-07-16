#include "color.h"
color_t color_inverted(color_t color) {
  return (color_t){.r = 1 - color.r, .g = 1 - color.g, .b = 1 - color.b, .a = color.a};
}

bool color_equals(color_t c1, color_t c2) {
  return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a;
}
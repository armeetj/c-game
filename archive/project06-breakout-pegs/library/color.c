#include "color.h"
rgb_color_t color_inverted(rgb_color_t color) {
  return (rgb_color_t){.r = 1 - color.r, .g = 1 - color.g, .b = 1 - color.b};
}
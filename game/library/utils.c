#include "utils.h"

double rand_range(double low, double high)
{
  return ((double)rand() * (high - low)) / (double)RAND_MAX + low;
}

color_t rgba(float r, float g, float b, float a)
{
  return (color_t){r / 255, g / 255, b / 255, a / 255};
}

color_t rand_color(double alpha)
{
  color_t color = rainbow_color(rand_range(0, 1000), 1000);
  return (color_t){color.r, color.g, color.b, alpha};
}

color_t rainbow_color(double n, double m)
{
  double a = ((5 * M_PI * n) / (3 * m)) + (M_PI / 2);
  double y = sin(a) * 192 + 128;
  double r = fmax(0, fmin(255, y));
  y = sin(a - 2 * M_PI / 3) * 192 + 128;
  double g = fmax(0, fmin(255, y));
  y = sin(a - 4 * M_PI / 3) * 192 + 128;
  double b = fmax(0, fmin(255, y));
  return (color_t){.r = r / 255, .g = g / 255, .b = b / 255, .a = 1};
}

list_t *make_circle(size_t num_points, size_t length, vector_t center)
{
  list_t *circle = list_init(num_points, free);
  double x_pos, y_pos;
  double increment_angle = 2 * M_PI / num_points;
  double angle = 0;
  for (uint32_t i = 0; i < num_points; i++)
  {
    x_pos = cos(angle) * length + center.x;
    y_pos = sin(angle) * length + center.y;
    vector_t *point = malloc(sizeof(vector_t));
    point->x = x_pos;
    point->y = y_pos;
    list_add(circle, point);
    angle += increment_angle;
  }
  return circle;
}

list_t *make_rectangle(double width, double height, vector_t center)
{
  list_t *rectangle = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){-0.5 * width + center.x, -0.5 * height + center.y};
  list_add(rectangle, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+0.5 * width + center.x, -0.5 * height + center.y};
  list_add(rectangle, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+0.5 * width + center.x, +0.5 * height + center.y};
  list_add(rectangle, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){-0.5 * width + center.x, +0.5 * height + center.y};
  list_add(rectangle, v);
  return rectangle;
}
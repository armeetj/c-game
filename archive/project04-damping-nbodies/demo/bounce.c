#include "body.h"
#include "color.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// body definition
typedef struct body {
  list_t *pts;
  vector_t vel;
  double rot;
  double rot_velocity;
  rgb_color_t color;
} body_t;

// state definition
typedef struct state {
  list_t *bodies;
} state_t;

// window constants
const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};
const vector_t MIN_POSITION = (vector_t){.x = 0, .y = 0};

// circle constants
const size_t NUM_STAR_POINTS = 5;
const size_t CIRCLE_RAD = 99;
const double START_ANGLE = 0;

// ratio between star's outer to inner outer circle should be 3 as per geometry
const double STAR_RADIUS_RATIO = 3;

// star constants
const vector_t INIT_VELOCITY = (vector_t){.x = 1, .y = 1};
const double INIT_ROTATION = 0;
const double INIT_ROTATION_VELOCITY = 0.01;
const float INIT_STAR_R = 0.2;
const float INIT_STAR_G = 0.9;
const float INIT_STAR_B = 0.9;

double rand_range(double low, double high) {
  return ((double)rand() * (high - low)) / (double)RAND_MAX + low;
}

list_t *make_circle(size_t radius, size_t num_points, size_t center_x,
                    size_t center_y) {
  double curr_angle = START_ANGLE;
  double vert_angle = 2 * M_PI / num_points;
  double x;
  double y;
  list_t *vertices = list_init(num_points, free);
  assert(vertices != NULL);
  for (size_t i = 0; i < num_points; i++) {
    x = cos(curr_angle) * radius + center_x;
    y = sin(curr_angle) * radius + center_y;
    vector_t vec_ptr = (vector_t){.x = x, .y = y};
    vector_t *temp_vector = malloc(sizeof(vector_t));
    temp_vector->x = vec_ptr.x;
    temp_vector->y = vec_ptr.y;
    list_add(vertices, temp_vector);
    curr_angle += vert_angle;
  }
  return vertices;
}

list_t *make_star(size_t length, size_t center_x, size_t center_y,
                  uint32_t num_points) {
  list_t *star = list_init(num_points * 2, free);
  double x_large, y_large, x_small, y_small;
  double increment_angle = 2 * M_PI / num_points;
  double large_angle = START_ANGLE, small_angle = M_PI / num_points;
  for (uint32_t i = 0; i < num_points; i++) {
    x_large = cos(large_angle) * length + center_x;
    y_large = sin(large_angle) * length + center_y;
    x_small = cos(small_angle) * length / STAR_RADIUS_RATIO + center_x;
    y_small = sin(small_angle) * length / STAR_RADIUS_RATIO + center_y;
    vector_t vec_large = (vector_t){.x = x_large, .y = y_large};
    vector_t vec_small = (vector_t){.x = x_small, .y = y_small};
    vector_t *temp_large = malloc(sizeof(vector_t));
    vector_t *temp_small = malloc(sizeof(vector_t));
    temp_large->x = vec_large.x;
    temp_large->y = vec_large.y;
    temp_small->x = vec_small.x;
    temp_small->y = vec_small.y;
    list_add(star, temp_large);
    list_add(star, temp_small);
    small_angle += increment_angle;
    large_angle += increment_angle;
  }
  return star;
}

state_t *emscripten_init() {
  vector_t min = MIN_POSITION;
  vector_t max = WINDOW;
  sdl_init(min, max);
  state_t *state = malloc(sizeof(state_t));
  state->bodies = list_init(1, free);
  body_t *star = malloc(sizeof(body_t));
  star->pts = make_star(CIRCLE_RAD, CENTER.x, CENTER.y, NUM_STAR_POINTS);
  star->vel = INIT_VELOCITY;
  star->rot = INIT_ROTATION;
  star->rot_velocity = INIT_ROTATION_VELOCITY;
  star->color =
      (rgb_color_t){.r = INIT_STAR_R, .g = INIT_STAR_G, .b = INIT_STAR_B};
  list_add(state->bodies, star);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  body_t *star = list_get(state->bodies, 0);
  list_t *star_pts = star->pts;
  sdl_draw_polygon(star_pts, star->color);
  vector_t center = polygon_centroid(star_pts);
  if (center.x + CIRCLE_RAD >= WINDOW.x ||
      center.x - CIRCLE_RAD <= MIN_POSITION.x) {
    star->vel.x = -star->vel.x;
    star->color = (rgb_color_t){
        .r = rand_range(0, 1), .g = rand_range(0, 1), .b = rand_range(0, 1)};
  }
  if (center.y + CIRCLE_RAD >= WINDOW.y ||
      center.y - CIRCLE_RAD <= MIN_POSITION.y) {
    star->vel.y = -star->vel.y;
    star->color = (rgb_color_t){
        .r = rand_range(0, 1), .g = rand_range(0, 1), .b = rand_range(0, 1)};
  }
  polygon_translate(star_pts, star->vel);
  polygon_rotate(star_pts, star->rot_velocity, center);
  sdl_show();
}

void emscripten_free(state_t *state) {
  list_free(state->bodies);
  free(state);
}
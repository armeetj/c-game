#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// window constants
const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};

// circle constants
const size_t NUM_STAR_POINTS = 5;
const size_t CIRCLE_RAD = 99;

// color constants
const float YELLOW_R = 1;
const float YELLOW_G = 1;
const float YELLOW_B = 0.0;

const double PI = 3.1415926535;

vec_list_t *make_star(size_t length, size_t center_x, size_t center_y,
                      uint32_t num_points) {
  vec_list_t *star = vec_list_init(num_points * 2);
  double x_large, y_large, x_small, y_small;
  double increment_angle = 2 * PI / num_points;
  double large_angle = 0, small_angle = increment_angle / 2;
  for (uint32_t i = 0; i < num_points; i++) {
    x_large = cos(large_angle) * length + center_x;
    y_large = sin(large_angle) * length + center_y;
    x_small = cos(small_angle) * length / 3 + center_x;
    y_small = sin(small_angle) * length / 3 + center_y;
    vector_t vec_large = (vector_t){.x = x_large, .y = y_large};
    vector_t vec_small = (vector_t){.x = x_small, .y = y_small};
    vector_t *temp_large = malloc(sizeof(vector_t));
    vector_t *temp_small = malloc(sizeof(vector_t));
    temp_large->x = vec_large.x;
    temp_large->y = vec_large.y;
    temp_small->x = vec_small.x;
    temp_small->y = vec_small.y;
    vec_list_add(star, temp_large);
    vec_list_add(star, temp_small);
    small_angle += increment_angle;
    large_angle += increment_angle;
  }
  return star;
}

// state definition (required in every demo)
// you can put anything in here!
// ideally only things that can change and are relevant to the main loop
typedef struct state {
  vec_list_t *star;
  vector_t pos;
  double d_theta;
  // size_t time_since_collision;
  // size_t curr_velocity_y;
  // double elasticity;
} state_t;

// initializes the state to hold the circle polygon and returns the state
state_t *emscripten_init() {
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);
  state_t *state1 = malloc(sizeof(state_t));
  state_t *state2 = malloc(sizeof(state_t));
  state1->star = make_star(CIRCLE_RAD, CENTER.x, CENTER.y, NUM_STAR_POINTS);
  state1->star = make_star(CIRCLE_RAD, CENTER.x, CENTER.y, NUM_STAR_POINTS);
  state1->pos.x = 1;
  state2->pos.x = 1;
  state1->pos.y = 1;
  state2->pos.y = 1;
  state1->d_theta = 0.01;
  state2->d_theta = 0.01;
  // state1->time_since_collision = 0;
  // state1->curr_velocity_y = 0;
  return state1, state2;
}

// runs each tick of the loop and shows the results
// takes the same state as initialized in emscripten_init
void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();

  vec_list_t *star = state->star;
  sdl_draw_polygon(star, YELLOW_R, YELLOW_G, YELLOW_B);
  vector_t center = polygon_centroid(star);

  if (center.x + CIRCLE_RAD >= WINDOW.x || center.x - CIRCLE_RAD <= 0) {
    state->pos.x = -state->pos.x;
  }

  if (center.y + CIRCLE_RAD >= WINDOW.y || center.y - CIRCLE_RAD <= 0) {
    state->pos.y = -state->pos.y;
  }

  polygon_translate(star, state->pos);
  polygon_rotate(star, state->d_theta, center);
  sdl_show();
}

// frees the memory associated with everything
void emscripten_free(state_t *state) {
  vec_list_t *star = state->star;
  free(star);
  free(state);
}

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
#include <time.h>

// body definition
typedef struct body {
  bool visible;
  list_t *pts;
  vector_t vel;
  double rot;
  double rot_velocity;
  double elasticity;
  rgb_color_t color;
} body_t;

// state def
typedef struct state {
  list_t *bodies;
  time_t start_time;
} state_t;

// window constants
const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};
const vector_t MIN_POSITION = (vector_t){.x = 0, .y = 0};

// circle constants
const size_t NUM_STAR_POINTS = 5;
const double STAR_RAD = 33;
const double STAR_RADIUS_RATIO = 3.0 / 7.0;

// demo props
const vector_t SPAWN = (vector_t){.x = 33, .y = 467};
const time_t SPAWN_TIME = 3;
const size_t NUM_SHAPES = 100; // how many shapes will be spawned in this demo
const size_t MIN_NUM_POINTS =
    5; // we want every star that spawns in to have >= 5 points
const double VEL_X = 0.5;
const vector_t GRAVITY = (vector_t){.x = 0, .y = -2};
const double ROT_DIRECTION = 0; // initial star orientation (rotation)
const double ROT_VELOCITY_MIN = 0.005;
const double ROT_VELOCITY_MAX = 0.02;
const double ELASTICITY_MIN = 0.8;
const double ELASTICITY_MAX = 0.99;

vector_t vector(double x, double y) { return (vector_t){.x = x, .y = y}; }

double rand_range(double low, double high) {
  return ((double)rand() * (high - low)) / (double)RAND_MAX + low;
}

list_t *make_star(size_t length, vector_t center, uint32_t num_points) {
  list_t *star = list_init(num_points * 2, free);
  double x_large, y_large, x_small, y_small;
  double increment_angle = 2 * M_PI / num_points;
  double large_angle = 0, small_angle = M_PI / num_points;
  for (uint32_t i = 0; i < num_points; i++) {
    x_large = cos(large_angle) * length + center.x;
    y_large = sin(large_angle) * length + center.y;
    x_small = cos(small_angle) * length * STAR_RADIUS_RATIO + center.x;
    y_small = sin(small_angle) * length * STAR_RADIUS_RATIO + center.y;
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
  state->start_time = time(NULL);
  state->bodies = list_init(NUM_SHAPES, free);

  for (size_t i = 0; i < NUM_SHAPES; i++) {
    body_t *new_body = malloc(sizeof(body_t));
    new_body->visible = false;
    new_body->pts =
        make_star(STAR_RAD, vector(SPAWN.x, SPAWN.y), MIN_NUM_POINTS + i);
    new_body->vel = vector(VEL_X, 0);
    new_body->rot = ROT_DIRECTION;
    double rotation_direction =
        pow(-1, (int16_t)rand_range(0, 2)); // positive = CW, negative = CCW
    new_body->rot_velocity =
        rotation_direction * rand_range(ROT_VELOCITY_MIN, ROT_VELOCITY_MAX);
    new_body->elasticity = rand_range(ELASTICITY_MIN, ELASTICITY_MAX);
    new_body->color = (rgb_color_t){
        .r = rand_range(0, 1), .g = rand_range(0, 1), .b = rand_range(0, 1)};
    list_add(state->bodies, new_body);
  }

  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  size_t time_elapsed = (size_t)time(NULL) - state->start_time;
  size_t index = time_elapsed / SPAWN_TIME;
  ((body_t *)list_get(state->bodies, index))->visible = true;
  double dt = time_since_last_tick();
  for (size_t i = 0; i < NUM_SHAPES; i++) {
    body_t *curr_body = (body_t *)list_get(state->bodies, i);
    if (curr_body->visible == true) {
      // draw shape
      sdl_draw_polygon(curr_body->pts, curr_body->color);
      list_t *pts = curr_body->pts;

      // translate shape
      polygon_translate(pts, curr_body->vel);

      // apply acceleration (gravity)
      curr_body->vel = vec_add(curr_body->vel, vec_multiply(dt, GRAVITY));

      // rotate shape
      vector_t center = polygon_centroid(pts);
      polygon_rotate(pts, curr_body->rot_velocity, center);

      // check if the shape is off the screen
      if (center.x - STAR_RAD >= WINDOW.x) {
        curr_body->visible = false;
        curr_body->color = (rgb_color_t){.r = rand_range(0, 1),
                                         .g = rand_range(0, 1),
                                         .b = rand_range(0, 1)};
      }

      if (center.y - STAR_RAD <= MIN_POSITION.y &&
          curr_body->vel.y < MIN_POSITION.y) {
        curr_body->vel.y = curr_body->vel.y * (-curr_body->elasticity);
        curr_body->color = (rgb_color_t){.r = rand_range(0, 1),
                                         .g = rand_range(0, 1),
                                         .b = rand_range(0, 1)};
      }
    }
  }
  sdl_show();
}

void emscripten_free(state_t *state) {
  list_free(state->bodies);
  free(state);
}
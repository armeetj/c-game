#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct rgb_color {
  float r;
  float g;
  float b;
} rgb_color_t;

typedef struct shape {
  bool visible;
  vec_list_t *pts;
  vector_t pos;
  vector_t vel;
  double rot;
  double rot_velocity;
  size_t time_since_collision;
  double elasticity;
  rgb_color_t color;
} shape_t;

// window constants
const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};

// circle constants
const size_t NUM_STAR_POINTS = 5;
const double CIRCLE_RAD = 33;

// color constants
const float YELLOW_R = 1;
const float YELLOW_G = 1;
const float YELLOW_B = 0.0;

// demo props
vector_t SPAWN = (vector_t){.x = 33, .y = 467};
const time_t SPAWN_TIME = 3;
const size_t NUM_SHAPES = 100;
const double VEL_X = 0.5;
const double PI = 3.1415926535;
const vector_t GRAVITY = (vector_t){.x = 0, .y = -2};
time_t start_time;

vector_t vector(double x, double y) { return (vector_t){.x = x, .y = y}; }

double randRange(double low, double high) {
  return ((double)rand() * (high - low)) / (double)RAND_MAX + low;
}

vec_list_t *make_star(size_t length, vector_t center, uint32_t num_points) {
  vec_list_t *star = vec_list_init(num_points * 2);
  double x_large, y_large, x_small, y_small;
  double increment_angle = 2 * PI / num_points;
  double large_angle = 0, small_angle = increment_angle / 2;
  for (uint32_t i = 0; i < num_points; i++) {
    x_large = cos(large_angle) * length + center.x;
    y_large = sin(large_angle) * length + center.y;
    x_small = cos(small_angle) * length * 4 / 7 + center.x;
    y_small = sin(small_angle) * length * 4 / 7 + center.y;
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

// initializes the state to hold the circle polygon and returns the state
shape_t **emscripten_init() {
  // initiaze window
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);
  shape_t **shapes = malloc(sizeof(shape_t *) * NUM_SHAPES);

  for (size_t i = 0; i < NUM_SHAPES; i++) {
    shape_t *new_shape = malloc(sizeof(shape_t));
    *new_shape =
        (shape_t){.visible = false,
                  .pts = make_star(CIRCLE_RAD, vector(SPAWN.x, SPAWN.y), 5 + i),
                  .pos = vector(SPAWN.x, SPAWN.y),
                  .vel = vector(VEL_X, 0),
                  .rot = 0.0,
                  .rot_velocity = 0.01,
                  .time_since_collision = 0.0,
                  .elasticity = randRange(0.8, 0.99),
                  .color = (rgb_color_t){.r = (float)rand() / (float)RAND_MAX,
                                         .g = (float)rand() / (float)RAND_MAX,
                                         .b = (float)rand() / (float)RAND_MAX}};
    shapes[i] = new_shape;
  }
  start_time = time(NULL);
  return shapes;
}

// SDL_Renderer *renderer;

// runs each tick of the loop and shows the results
// takes the same state as initialized in emscripten_init
void emscripten_main(shape_t **shapes) {
  sdl_clear();
  size_t time_elapsed = (size_t)time(NULL) - (size_t)start_time;
  size_t index = time_elapsed / SPAWN_TIME;
  shapes[index]->visible = true;
  double dt = time_since_last_tick();
  for (size_t i = 0; i < NUM_SHAPES; i++) {
    shape_t *curr_shape = shapes[i];
    if (curr_shape->visible == true) {
      // draw shape
      sdl_draw_polygon(curr_shape->pts, curr_shape->color.r,
                       curr_shape->color.g, curr_shape->color.b);
      vec_list_t *pts = curr_shape->pts;

      // translate shape
      polygon_translate(pts, curr_shape->vel);

      // apply acceleration (gravity)
      curr_shape->vel = vec_add(curr_shape->vel, vec_multiply(dt, GRAVITY));

      // rotate shape
      vector_t center = polygon_centroid(pts);
      polygon_rotate(pts, curr_shape->rot_velocity, center);

      // check if the shape is off the screen
      if (center.x - CIRCLE_RAD >= WINDOW.x) { // TODO: Add length of polygon
        curr_shape->visible = false;
      }

      if (center.y - CIRCLE_RAD <= 0 && curr_shape->vel.y < 0) {
        curr_shape->vel.y = curr_shape->vel.y * (-curr_shape->elasticity);
      }
    }
  }
  sdl_show();
}

// frees the memory associated with everything
void emscripten_free(shape_t **shapes) {
  for (int i = 0; i < NUM_SHAPES; i++) {
    free(shapes[i]->pts);
    free(shapes[i]);
  }

  free(shapes);
}

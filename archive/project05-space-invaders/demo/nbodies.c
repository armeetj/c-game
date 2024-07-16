#include "body.h"
#include "color.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// state def
typedef struct state {
  scene_t *scene;
  double time_s;
} state_t;

// window constants
const vector_t WINDOW = (vector_t){.x = 1600, .y = 900};
const vector_t CENTER = (vector_t){.x = 800, .y = 450};
const vector_t MIN_POSITION = (vector_t){.x = 0, .y = 0};

// diamond constants
const size_t NUM_DIAMONDS = 60;
const size_t NUM_POINTS = 4;
const double DIAMOND_RATIO = 3.0 / 7.0;
const double DIAMOND_SIZE = 30;
const double DIAMOND_MASS = 1;
const double COLOR_SCALE_FACTOR = 3.5;

const double GRAVITY_CONSTANT = 1e5;

// helper functions
double rand_range(double low, double high) {
  return ((double)rand() * (high - low)) / (double)RAND_MAX + low;
}

vector_t vector(double x, double y) { return (vector_t){.x = x, .y = y}; }

list_t *make_diamond(size_t length, vector_t center) {
  size_t num_edges = NUM_POINTS * 2;
  list_t *diamond = list_init(num_edges, free);
  double x_pos, y_pos;
  double increment_angle = 2 * M_PI / num_edges;
  double angle = 0;
  for (uint32_t i = 0; i < num_edges; i++) {
    if (i % 2 == 0) {
      x_pos = cos(angle) * length + center.x;
      y_pos = sin(angle) * length + center.y;
    } else {
      x_pos = cos(angle) * length * DIAMOND_RATIO + center.x;
      y_pos = sin(angle) * length * DIAMOND_RATIO + center.y;
    }
    vector_t *point = malloc(sizeof(vector_t));
    point->x = x_pos;
    point->y = y_pos;
    list_add(diamond, point);
    angle += increment_angle;
  }
  return diamond;
}

void key_press_handler(state_t *state, char key, key_event_type_t type,
                       double held_time) {
  switch (key) {
  // dev mode with "d" key
  case 100:
    if (type == 0) {
      scene_set_dev_mode(state->scene, true);
    } else {
      scene_set_dev_mode(state->scene, false);
    }
    break;
  }
}

state_t *emscripten_init() {
  // init sdl
  sdl_init(MIN_POSITION, WINDOW);
  // printf("%s\n", "To toggle dev-mode (view all acceleration vectors), hold
  // the D key");

  // init state
  state_t *state = malloc(sizeof(state_t));
  state->time_s = 0.0;
  state->scene = scene_init();

  // add diamonds
  for (size_t i = 0; i < NUM_DIAMONDS; i++) {
    vector_t position = vector(rand_range(MIN_POSITION.x, WINDOW.x),
                               rand_range(MIN_POSITION.y, WINDOW.y));
    double size = DIAMOND_SIZE;
    double mass = DIAMOND_MASS;
    rgb_color_t *color = malloc(sizeof(rgb_color_t));
    double offset = rand_range(0, M_PI / 2);
    color->r = fabs(sin(offset));
    color->g = fabs(sin(offset + (M_PI / 6)));
    color->b = fabs(sin(offset + (M_PI / 4)));
    body_t *diamond = body_init(make_diamond(size, position), mass, *color);
    rgb_color_t *dev_mode_color = malloc(sizeof(rgb_color_t));
    *dev_mode_color = (rgb_color_t){.r = 0, .g = 0, .b = 0};
    rgb_color_t *color2 = (rgb_color_t *)body_get_info(diamond);
    color2 = dev_mode_color;
    body_set_position(diamond, position);
    scene_add_body(state->scene, diamond);
  }

  // add all forces between every pair of bodies
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    for (size_t j = i + 1; j < scene_bodies(state->scene); j++) {
      body_t *body1 = scene_get_body(state->scene, i);
      body_t *body2 = scene_get_body(state->scene, j);
      create_newtonian_gravity(state->scene, GRAVITY_CONSTANT, body1, body2);
    }
  }
  return state;
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  state->time_s += dt;
  sdl_clear();
  sdl_on_key(key_press_handler); // place this after sdl_clear();
  scene_tick_canon_no_reset(state->scene, dt);
  scene_t *scene = state->scene;
  if (scene_get_dev_mode(state->scene)) {
    double max_accel = 1200;
    for (size_t i = 0; i < scene_bodies(scene); i++) {
      body_t *body = scene_get_body(scene, i);
      double curr_accel = vec_norm(body_get_acceleration(body));
      double r = 0;
      if (curr_accel == 0) {
        curr_accel = max_accel;
      }
      r = fmin(1, COLOR_SCALE_FACTOR * log((curr_accel) / (max_accel) + 1));
      double g = 0;
      double b = 1 - r;
      if (curr_accel != 0) {
        body_set_color(body, (rgb_color_t){.r = r, .g = g, .b = b});
      }
    }
  } else {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
      body_t *body = scene_get_body(scene, i);
      body_set_color(body, *((rgb_color_t *)body_get_info(body)));
    }
  }

  scene_draw(state->scene);
  scene_accel_reset(state->scene);
  sdl_show();
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
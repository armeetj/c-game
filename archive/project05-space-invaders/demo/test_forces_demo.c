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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// state def
typedef struct state {
  scene_t *scene;
} state_t;

// window constants
const vector_t WINDOW = (vector_t){.x = 1600, .y = 900};
const vector_t CENTER = (vector_t){.x = 800, .y = 450};
const vector_t MIN_POSITION = (vector_t){.x = 0, .y = 0};

// actual constants
const double M = 10;
const double K = 2;
const double A = 900;
const double DT = 1e-6;
const int STEPS = 1000000;

// helpers

list_t *make_shape() {
  list_t *shape = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){-10, -10};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+10, -10};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+10, +10};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){-10, +10};
  list_add(shape, v);
  return shape;
}

// emscripten: init
state_t *emscripten_init() {
  // init sdl
  sdl_init(MIN_POSITION, WINDOW);

  // init state
  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();

  // add earth body (to simulate gravity)
  body_t *mass = body_init(make_shape(), M, (rgb_color_t){1, 1, 1});
  body_set_centroid(mass, (vector_t){A, 450});
  scene_add_body(state->scene, mass);

  body_t *anchor = body_init(make_shape(), INFINITY, (rgb_color_t){1, 1, 0});
  body_set_centroid(anchor, CENTER);
  scene_add_body(state->scene, anchor);

  create_spring(state->scene, K, mass, anchor);

  return state;
}

// emscripten: main engine loop
void emscripten_main(state_t *state) {
  // sdl: clear window
  sdl_clear();

  // update scene & bodies
  scene_tick(state->scene, DT * 1e5);

  // draws all the bodies in a scene
  scene_draw(state->scene);

  // sdl: show
  sdl_show();
}

// emscripten: free resources
void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}

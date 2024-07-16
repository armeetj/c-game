#include "body.h"
#include "collision.h"
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
const vector_t PLAYER_MOVE_SPEED = (vector_t){.x = 120, .y = 0};

// actual constants
const double M = 10;
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

void key_press_handler(state_t *state, char key, key_event_type_t type,
                       double held_time) {
  if (scene_bodies(state->scene) != 0) {
    body_t *player = scene_get_body(state->scene, 0);
    // player controls
    if (type == 0) {
      switch (key) {
      case 'w':
        body_set_velocity(player, (vector_t){.x = 0, .y = 200});
        break;
      case 's':
        body_set_velocity(player, (vector_t){.x = 0, .y = -200});
        break;
      case 'd':
        body_set_velocity(player, PLAYER_MOVE_SPEED);
        break;
      case 'a':
        body_set_velocity(
            player,
            vec_negate(PLAYER_MOVE_SPEED)); // works because y-velocity = 0
        break;
      }
    }
    // the following is to fix an issue where leaving a key while holdnig
    // another causes stopping.
    else if (type == 1 &&
             vec_equals(body_get_velocity(player), PLAYER_MOVE_SPEED)) {
      switch (key) {
      case 'd':
        body_set_velocity(player, VEC_ZERO);
        break;
      }
    } else if (type == 1 && vec_equals(body_get_velocity(player),
                                       vec_negate(PLAYER_MOVE_SPEED))) {
      switch (key) {
      case 'a':
        body_set_velocity(player, VEC_ZERO);
        break;
      }
    }
  }
}

// emscripten: init
state_t *emscripten_init() {
  // init sdl
  sdl_init(MIN_POSITION, WINDOW);

  // init state
  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();

  body_t *moving_body = body_init(make_shape(), M, (rgb_color_t){1, 1, 1});
  body_set_centroid(moving_body, (vector_t){A, 450});
  scene_add_body(state->scene, moving_body);

  body_t *still_body =
      body_init(make_shape(), INFINITY, (rgb_color_t){1, 1, 0});
  body_set_centroid(still_body, CENTER);
  scene_add_body(state->scene, still_body);

  create_destructive_collision(state->scene, scene_get_body(state->scene, 0),
                               scene_get_body(state->scene, 1));

  return state;
}

// emscripten: main engine loop
void emscripten_main(state_t *state) {

  // sdl: clear window
  sdl_clear();

  sdl_on_key(key_press_handler);

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

#include "body.h"
#include "color.h"
#include "list.h"
#include "forces.h"
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

// rope constants
const rgb_color_t ROPE_COLOR = (rgb_color_t) {.r = 1, .g = 1, .b = 1};
const vector_t ROPE_START_POS = (vector_t){.x = 500, .y = 800};
const size_t ROPE_NUM_BODIES = 100;
const double ROPE_SEGMENT_MASS = 0.5;
const double ROPE_LENGTH = 600;
const double ROPE_K_CONST = 2000;
const double ROPE_SEGMENT_RESOLUTION = 100;
const double ROPE_DAMPING_CONSTANT = 0.2;

// earth constants
const double GRAVITY_CONST = 1e-10;
const double EARTH_MASS = 8e26;
const vector_t EARTH_POSITION = (vector_t){.x = 800, .y = -1e7};
const rgb_color_t EARTH_COLOR = (rgb_color_t){.r = 0, .g = 0, .b = 0};
const double EARTH_RADIUS = 1;
const double EARTH_NUM_POINTS = 3;

const double TICK_TIME = 0.01;

// helper functions
double rand_range(double low, double high) {
  return ((double)rand() * (high - low)) / (double)RAND_MAX + low;
}

vector_t vector(double x, double y) { return (vector_t){.x = x, .y = y}; }


list_t *make_pixel(vector_t position) {
  list_t *pts = list_init(4, free);
  vector_t *lower_left = malloc(sizeof(vector_t));
  lower_left->x = position.x - 1;
  lower_left->y = position.y - 1;
  list_add(pts, lower_left);

  vector_t *upper_left = malloc(sizeof(vector_t));
  upper_left->x = position.x - 1;
  upper_left->y = position.y + 1;
  list_add(pts, upper_left);

  vector_t *upper_right = malloc(sizeof(vector_t));
  upper_right->x = position.x + 1;
  upper_right->y = position.y + 1;
  list_add(pts, upper_right);

  vector_t *lower_right = malloc(sizeof(vector_t));
  lower_right->x = position.x + 1;
  lower_right->y = position.y - 1;
  list_add(pts, lower_right);
  return pts;
}


list_t *make_circle(size_t num_points, size_t length, vector_t center) {
  list_t *circle = list_init(num_points, free);
  if(length <= 1) {
    return make_pixel(center);
  }
  double x_pos, y_pos;
  double increment_angle = 2 * M_PI / num_points;
  double angle = 0;
  for (uint32_t i = 0; i < num_points; i++) {
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

// emscripten: init
state_t *emscripten_init() {
  // init sdl
  sdl_init(MIN_POSITION, WINDOW);
  
  // init state
  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();

  // add earth body (to simulate gravity)
  list_t *earth_pts = make_circle(EARTH_NUM_POINTS, EARTH_RADIUS, EARTH_POSITION);
  body_t *earth = body_init(earth_pts, EARTH_MASS, EARTH_COLOR);
  body_set_position(earth, EARTH_POSITION);
  scene_add_body(state->scene, earth);

  // add rope segments
  vector_t center = ROPE_START_POS;
  for(size_t t = 0; t < ROPE_NUM_BODIES; t++) {
    list_t *circle_pts = make_circle(ROPE_SEGMENT_RESOLUTION, ROPE_LENGTH / (ROPE_NUM_BODIES * 2), center);
    body_t *circle = body_init(circle_pts, ROPE_SEGMENT_MASS, ROPE_COLOR);
    body_set_position(circle, center);
    scene_add_body(state->scene, circle);
    center.x += ROPE_LENGTH / ROPE_NUM_BODIES;
  }

  // add forces
  size_t num_bodies = scene_bodies(state->scene);
  for(size_t i = 1; i < num_bodies - 1; i++) {
      body_t *body1 = scene_get_body(state->scene, i);
      body_t *body2 = scene_get_body(state->scene, i + 1);
      if(i == 1) {
        create_spring(state->scene, ROPE_K_CONST, body1, body2); // body 2 is affected by body 1
      } 
      else if(i == num_bodies - 2) {
        create_newtonian_gravity(state->scene, GRAVITY_CONST, body1, earth);
        create_drag(state->scene, ROPE_DAMPING_CONSTANT, body1);
        create_spring(state->scene, ROPE_K_CONST, body2, body1);
      }
       else {
        create_newtonian_gravity(state->scene, GRAVITY_CONST, body1, earth);
        create_spring(state->scene, ROPE_K_CONST, body1, body2);
        create_spring(state->scene, ROPE_K_CONST, body2, body1);
        create_drag(state->scene, ROPE_DAMPING_CONSTANT, body1);
      }
    }
    return state;
}

void key_press_handler(state_t *state, char key, key_event_type_t type, double held_time) {
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

// emscripten: main engine loop
void emscripten_main(state_t *state) {
  // sdl: clear window
  sdl_clear();

  // update dt
  double dt = time_since_last_tick();

  vector_t earth_pos = body_get_position(scene_get_body(state->scene, 0));

  // handle keypresses
  sdl_on_key(key_press_handler);
  // while (!sdl_is_done(state));

  // update scene & bodies
  scene_tick(state->scene, TICK_TIME);

  // draws all the bodies in a scene
  scene_draw(state->scene);
  
  // // reset all accelerations to be calculated in the next tick
  scene_accel_reset(state->scene);

  // sdl: show
  sdl_show();
}

// emscripten: free resources
void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}

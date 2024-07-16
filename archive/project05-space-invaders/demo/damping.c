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
  double pacman_dir;
  double time_s;
} state_t;

// window constants
const vector_t WINDOW = (vector_t){.x = 1600, .y = 900};
const vector_t CENTER = (vector_t){.x = 800, .y = 450};
const vector_t MIN_POSITION = (vector_t){.x = 0, .y = 0};

// rope constants
const rgb_color_t ROPE_COLOR = (rgb_color_t){.r = 1, .g = 1, .b = 1};
const vector_t ROPE_START_POS = (vector_t){.x = 500, .y = 800};
const size_t ROPE_NUM_BODIES = 100;
const double ROPE_SEGMENT_MASS = 0.5;
const double ROPE_LENGTH = 600;
const double ROPE_K_CONST = 2300;
const double ROPE_SEGMENT_RESOLUTION = 100;
const double ROPE_DAMPING_CONSTANT = 0.94;

// earth constants
const double GRAVITY_CONST = 1e-10;
const double EARTH_MASS = 8e26;
const vector_t EARTH_POSITION = (vector_t){.x = 800, .y = -1e7};
const rgb_color_t EARTH_COLOR = (rgb_color_t){.r = 0, .g = 0, .b = 0};
const double EARTH_RADIUS = 1;
const double EARTH_NUM_POINTS = 3;

const double TICK_TIME = 0.01;

// pacman constants
const rgb_color_t PACMAN_INIT_COLOR = (rgb_color_t){.r = 1, .g = 1, .b = 0};
const size_t PACMAN_RESOLUTION = 100;
const vector_t PACMAN_INIT_VEL = (vector_t){.x = 0, .y = 0};
const double PACMAN_INIT_DIR = 0;
const double PACMAN_INIT_SIZE = 30;
const double PACMAN_BASE_VEL = 200;
const double PACMAN_MOUTH_OPEN_ANGLE = M_PI / 3;
const double PACMAN_MOUTH_CLOSED_ANGLE = 0;
const double PACMAN_CHOMP_FREQUENCY = 15;
const double PACMAN_ACCELERATION = 300;
const double PACMAN_GROW_FACTOR = 1;

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

list_t *make_pacman(size_t radius, vector_t center, double direction,
                    double mouth_angle, uint32_t num_points) {
  double curr_angle = mouth_angle / 2;
  double total_angle = 2 * M_PI - mouth_angle;
  double increment_angle = total_angle / num_points;
  double x;
  double y;
  list_t *pts = list_init(num_points, free);
  for (size_t i = 0; i < num_points; i++) {
    x = cos(curr_angle) * radius + center.x;
    y = sin(curr_angle) * radius + center.y;
    vector_t *pt = malloc(sizeof(vector_t));
    pt->x = x;
    pt->y = y;
    list_add(pts, pt);
    curr_angle += increment_angle;
  }

  increment_angle = mouth_angle / 3;
  curr_angle = 2 * M_PI - (mouth_angle / 2) + increment_angle;
  vector_t *vec_origin = malloc(sizeof(vector_t));
  vec_origin->x = center.x;
  vec_origin->y = center.y;
  if (mouth_angle != 0) {
    list_add(pts, vec_origin);
  }
  polygon_rotate(pts, direction, center);
  return pts;
}

list_t *make_circle(size_t num_points, size_t length, vector_t center) {
  list_t *circle = list_init(num_points, free);
  if (length <= 1) {
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

  state->pacman_dir = 0;
  state->time_s = 0;

  // add pacman to cut rope
  body_t *pacman =
      body_init(make_pacman(PACMAN_INIT_SIZE, CENTER, 0,
                            PACMAN_MOUTH_OPEN_ANGLE, PACMAN_RESOLUTION),
                1, PACMAN_INIT_COLOR);
  body_set_velocity(pacman, PACMAN_INIT_VEL);
  body_set_centroid(pacman, CENTER);
  body_set_rotation(pacman, PACMAN_INIT_DIR);
  scene_add_body(state->scene, pacman);

  // add earth body (to simulate gravity)
  list_t *earth_pts =
      make_circle(EARTH_NUM_POINTS, EARTH_RADIUS, EARTH_POSITION);
  body_t *earth = body_init(earth_pts, EARTH_MASS, EARTH_COLOR);
  body_set_centroid(earth, EARTH_POSITION);
  scene_add_body(state->scene, earth);

  // add rope segments
  vector_t center = ROPE_START_POS;
  for (size_t t = 0; t < ROPE_NUM_BODIES; t++) {
    list_t *circle_pts = make_circle(
        ROPE_SEGMENT_RESOLUTION, ROPE_LENGTH / (ROPE_NUM_BODIES * 2), center);
    body_t *circle = body_init(circle_pts, ROPE_SEGMENT_MASS, ROPE_COLOR);
    body_set_centroid(circle, center);
    scene_add_body(state->scene, circle);
    center.x += ROPE_LENGTH / ROPE_NUM_BODIES;
  }

  // add forces
  size_t num_bodies = scene_bodies(state->scene);
  for (size_t i = 2; i < num_bodies - 1; i++) {
    body_t *body1 = scene_get_body(state->scene, i);
    body_t *body2 = scene_get_body(state->scene, i + 1);
    if (i == 2) {
      create_spring(state->scene, ROPE_K_CONST, body2,
                    body1); // body 2 is affected by body 1
    } else if (i == num_bodies - 2) {
      create_newtonian_gravity(state->scene, GRAVITY_CONST, body1, earth);
      create_drag(state->scene, ROPE_DAMPING_CONSTANT, body1);
      create_spring(state->scene, ROPE_K_CONST, body1, body2);
    } else {
      create_newtonian_gravity(state->scene, GRAVITY_CONST, body1, earth);
      create_spring(state->scene, ROPE_K_CONST, body1, body2);
      create_spring(state->scene, ROPE_K_CONST, body2, body1);
      create_drag(state->scene, ROPE_DAMPING_CONSTANT, body1);
    }
  }
  return state;
}

void key_press_handler(state_t *state, char key, key_event_type_t type,
                       double held_time) {
  switch (key) {
  // dev mode with "d" key
  case 'e':
    if (type == 0) {
      scene_set_dev_mode(state->scene, true);
    } else {
      scene_set_dev_mode(state->scene, false);
    }
    break;
  }
  body_t *pacman = scene_get_body(state->scene, 0);
  if (type == 0) {
    switch (key) {
    case 1:
    case 97:
      body_set_velocity(
          pacman,
          vector(-PACMAN_BASE_VEL - held_time * PACMAN_ACCELERATION, 0));
      body_set_rotation(pacman, M_PI);
      state->pacman_dir = M_PI;
      break;
    case 2:
    case 119:
      body_set_velocity(
          pacman, vector(0, PACMAN_BASE_VEL + held_time * PACMAN_ACCELERATION));
      body_set_rotation(pacman, M_PI / 2);
      state->pacman_dir = M_PI / 2;
      break;
    case 3:
    case 100:
      body_set_velocity(
          pacman, vector(PACMAN_BASE_VEL + held_time * PACMAN_ACCELERATION, 0));
      body_set_rotation(pacman, 0);
      state->pacman_dir = 0;
      break;
    case 4:
    case 115:
      body_set_velocity(pacman, vector(0, -PACMAN_BASE_VEL -
                                              held_time * PACMAN_ACCELERATION));
      body_set_rotation(pacman, 3 * M_PI / 2);
      state->pacman_dir = 3 * M_PI / 2;
      break;
    }
  } else {
    if (body_get_velocity(pacman).x < 0) {
      body_set_velocity(pacman, vector(-PACMAN_BASE_VEL, 0));
    } else if (body_get_velocity(pacman).x > 0) {
      body_set_velocity(pacman, vector(PACMAN_BASE_VEL, 0));
    } else if (body_get_velocity(pacman).y < 0) {
      body_set_velocity(pacman, vector(0, -PACMAN_BASE_VEL));
    } else {
      body_set_velocity(pacman, vector(0, PACMAN_BASE_VEL));
    }
  }
}

// emscripten: main engine loop
void emscripten_main(state_t *state) {
  state->time_s += time_since_last_tick();

  // sdl: clear window
  sdl_clear();

  // handle keypresses
  sdl_on_key(key_press_handler);
  // while (!sdl_is_done(state));

  // pacman wrap around
  body_t *pacman = scene_get_body(state->scene, 0);
  if (body_get_centroid(pacman).x > WINDOW.x) {
    vector_t new_centroid = body_get_centroid(pacman);
    new_centroid.x = 0;
    body_set_centroid(pacman, new_centroid);
  } else if (body_get_centroid(pacman).x < 0) {
    vector_t new_centroid = body_get_centroid(pacman);
    new_centroid.x = WINDOW.x;
    body_set_centroid(pacman, new_centroid);
  }
  if (body_get_centroid(pacman).y > WINDOW.y) {
    vector_t new_centroid = body_get_centroid(pacman);
    new_centroid.y = 0;
    body_set_centroid(pacman, new_centroid);
  } else if (body_get_centroid(pacman).y < 0) {
    vector_t new_centroid = body_get_centroid(pacman);
    new_centroid.y = WINDOW.y;
    body_set_centroid(pacman, new_centroid);
  }
  for (size_t i = 2; i < scene_bodies(state->scene); i++) {
    body_t *rope_segment = scene_get_body(state->scene, i);
    if (vec_dist(body_get_centroid(rope_segment), body_get_centroid(pacman)) <
        PACMAN_INIT_SIZE) {
      scene_remove_forces_from_body(state->scene, rope_segment);
      scene_remove_body(state->scene, i);
      i--;
    }
  }

  // update scene & bodies canonically
  scene_tick_canon_no_reset(state->scene, TICK_TIME);
  scene_draw(state->scene);

  scene_accel_reset(state->scene);

  // sdl: show
  sdl_show();
}

// emscripten: free resources
void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}

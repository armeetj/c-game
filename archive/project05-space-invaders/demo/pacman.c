#include "body.h"
#include "color.h"
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
  bool game_started;
  scene_t *scene;
  size_t pacman_size;
  double pacman_dir;
  rgb_color_t pacman_color;
  double time_s;
  double time_since_spawn;
} state_t;

// window constants
const vector_t WINDOW = (vector_t){.x = 1600, .y = 900};
const vector_t CENTER = (vector_t){.x = 800, .y = 450};
const vector_t MIN_POSITION = (vector_t){.x = 0, .y = 0};

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

// food constants
const double FOOD_MIN_SIZE_FACTOR = 0.5;
const double FOOD_MAX_SIZE_FACTOR = 1;
const size_t FOOD_COUNT_INITIAL = 13;
const double FOOD_SIDE_LENGTH = 10;
const time_t FOOD_SPAWN_TIME = 1;
const time_t FOOD_FLASH_FREQUENCY = 1;
const rgb_color_t FOOD_COLOR = (rgb_color_t){.r = 1, .g = 0.72, .b = 0.69};

// misc constants
const size_t PTS_IN_PELLET = 4;

double rand_range(double low, double high) {
  return ((double)rand() * (high - low)) / (double)RAND_MAX + low;
}

vector_t vector(double x, double y) { return (vector_t){.x = x, .y = y}; }

list_t *make_pellet(size_t length, vector_t position) {
  list_t *pts = list_init(PTS_IN_PELLET, free);
  vector_t *lower_left = malloc(sizeof(vector_t));
  lower_left->x = position.x - length / 2;
  lower_left->y = position.y - length / 2;
  list_add(pts, lower_left);

  vector_t *upper_left = malloc(sizeof(vector_t));
  upper_left->x = position.x - length / 2;
  upper_left->y = position.y + length / 2;
  list_add(pts, upper_left);

  vector_t *upper_right = malloc(sizeof(vector_t));
  upper_right->x = position.x + length / 2;
  upper_right->y = position.y + length / 2;
  list_add(pts, upper_right);

  vector_t *lower_right = malloc(sizeof(vector_t));
  lower_right->x = position.x + length / 2;
  lower_right->y = position.y - length / 2;
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

state_t *emscripten_init() {
  vector_t min = MIN_POSITION;
  vector_t max = WINDOW;
  sdl_init(min, max);
  state_t *state = malloc(sizeof(state_t));
  state->time_since_spawn = 0;
  state->game_started = false;
  state->scene = scene_init();
  state->pacman_color = PACMAN_INIT_COLOR;
  state->pacman_size = PACMAN_INIT_SIZE;
  state->pacman_dir = 0;
  state->time_s = 0;
  body_t *pacman =
      body_init(make_pacman(state->pacman_size, CENTER, 0,
                            PACMAN_MOUTH_OPEN_ANGLE, PACMAN_RESOLUTION),
                1, state->pacman_color);
  body_set_velocity(pacman, PACMAN_INIT_VEL);
  body_set_centroid(pacman, CENTER);
  body_set_rotation(pacman, PACMAN_INIT_DIR);
  scene_add_body(state->scene, pacman);

  for (size_t i = 0; i < FOOD_COUNT_INITIAL; i++) {
    vector_t pellet_pos = vector(rand_range(MIN_POSITION.x, WINDOW.x),
                                 rand_range(MIN_POSITION.y, WINDOW.y));
    body_t *food = body_init(make_pellet(FOOD_SIDE_LENGTH, pellet_pos), 1,
                             (rgb_color_t){.r = rand_range(0, 1),
                                           rand_range(0, 1),
                                           rand_range(0, 1)});
    body_set_centroid(food, pellet_pos);
    scene_add_body(state->scene, food);
  }
  return state;
}

void key_press_handler(state_t *state, char key, key_event_type_t type,
                       double held_time) {
  if (!state->game_started) {
    state->game_started = true;
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

double distance_between(vector_t v1, vector_t v2) {
  double result = sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2));
  return result;
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  state->time_s += dt;
  sdl_clear();
  sdl_on_key(key_press_handler);
  // while (!sdl_is_done(state));

  state->time_since_spawn += dt;
  if (state->time_since_spawn >= FOOD_SPAWN_TIME) {
    state->time_since_spawn = 0;
    vector_t pellet_pos = vector(rand_range(MIN_POSITION.x, WINDOW.x),
                                 rand_range(MIN_POSITION.y, WINDOW.y));
    body_t *food = body_init(make_pellet(FOOD_SIDE_LENGTH, pellet_pos), 1,
                             (rgb_color_t){.r = rand_range(0, 1),
                                           rand_range(0, 1),
                                           rand_range(0, 1)});
    body_set_centroid(food, pellet_pos);
    scene_add_body(state->scene, food);
  }

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

  double mouth_angle = PACMAN_MOUTH_OPEN_ANGLE;
  double food_size = FOOD_SIDE_LENGTH;
  if (state->game_started) {
    // triangle wave (chomping animation)
    mouth_angle =
        fmax(PACMAN_MOUTH_CLOSED_ANGLE,
             (PACMAN_MOUTH_OPEN_ANGLE)*abs(
                 (int32_t)(state->time_s * PACMAN_CHOMP_FREQUENCY) % 2 - 1));
  }
  list_t *pacman_pts = body_get_shape(pacman);
  pacman_pts = make_pacman(state->pacman_size, body_get_centroid(pacman),
                           state->pacman_dir, mouth_angle, PACMAN_RESOLUTION);

  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    body_t *pellet = scene_get_body(state->scene, i);
    if (distance_between(body_get_centroid(pellet), body_get_centroid(pacman)) <
        state->pacman_size) {
      state->pacman_color = body_get_color(scene_get_body(state->scene, i));
      scene_remove_body(state->scene, i);
      i--;
      state->pacman_size += PACMAN_GROW_FACTOR;
    }
  }

  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    // stagger animation by index using sin(... + i)
    food_size = FOOD_SIDE_LENGTH *
                fmax(FOOD_MIN_SIZE_FACTOR,
                     FOOD_MAX_SIZE_FACTOR *
                         fabs(sin(state->time_s * FOOD_FLASH_FREQUENCY + i)));
    body_t *pellet = scene_get_body(state->scene, i);
    list_t *pellet_pts = body_get_shape(pellet);
    // use a random food size to stagger blinking animation
    pellet_pts = make_pellet(food_size, body_get_centroid(pellet));
    sdl_draw_polygon(pellet_pts, body_get_color(pellet));
  }

  scene_tick(state->scene, dt);
  sdl_draw_polygon(pacman_pts, state->pacman_color);
  sdl_show();
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
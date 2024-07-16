// includes
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
#include <string.h>
#include <time.h>

// state def
typedef struct state {
  scene_t *scene;
  double time_s;
  double time_since_player_shot;
  double time_since_invader_shot;
  double num_invaders_shot;
  bool bullet_present;
} state_t;

// window constants
const vector_t WINDOW = (vector_t){.x = 672, .y = 768};
const vector_t CENTER = (vector_t){.x = 336, .y = 384};
const vector_t MIN_POSITION = (vector_t){.x = 0, .y = 0};

// invader constants
const double PADDING_X = 15;
const double PADDING_Y = 10;
const double NUM_INVADERS_PER_ROW = 10;
const double ROWS = 3;
const double ROW_SIZE = 600;
const vector_t INVADER_VEL = (vector_t){.x = 100, .y = 0};
const double INVADER_MASS = 1;
const double INVADER_RESOLUTION = 100;
const rgb_color_t INVADER_COLOR = (rgb_color_t){.r = 1, .g = 1, .b = 1};
const size_t INVADER_INFO_LENGTH = 8;

// player bullet constants
const double BULLET_WIDTH = 3;
const double BULLET_LENGTH = 5;
const double BULLET_MASS = 1;
const rgb_color_t BULLET_COLOR = (rgb_color_t){.r = .7, .g = .3, .b = .5};
const vector_t BULLET_VEL = (vector_t){.x = 0, .y = 400};
const double BULLET_COOLDOWN = 1;
const size_t PLAYER_BULLET_INFO_LENGTH = 14;

// invader bullet constants
const size_t INVADER_BULLET_INFO_LENGTH = 15;

// player constants
const double PLAYER_LENGTH = 20;
const double PLAYER_WIDTH = 40;
const vector_t PLAYER_MOVE_SPEED = (vector_t){.x = 120, .y = 0};
const double PLAYER_RESOLUTION = 100;
const double PLAYER_PADDING = 20;
const vector_t PLAYER_SPAWN = (vector_t){384, 30};
const double PLAYER_MASS = 10;
const rgb_color_t PLAYER_COLOR = (rgb_color_t){.r = 0, .g = 0, .b = 1};
const size_t PLAYER_INFO_LENGTH = 7;
const double DT = 1e-1;

// helper functions
double rand_range(double low, double high) {
  return ((double)rand() * (high - low)) / (double)RAND_MAX + low;
}

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

list_t *make_pixel() {
  list_t *pts = list_init(4, free);
  vector_t *lower_left = malloc(sizeof(vector_t));
  lower_left->x = -1;
  lower_left->y = -1;
  list_add(pts, lower_left);

  vector_t *upper_left = malloc(sizeof(vector_t));
  upper_left->x = -1;
  upper_left->y = 1;
  list_add(pts, upper_left);

  vector_t *upper_right = malloc(sizeof(vector_t));
  upper_right->x = 1;
  upper_right->y = 1;
  list_add(pts, upper_right);

  vector_t *lower_right = malloc(sizeof(vector_t));
  lower_right->x = 1;
  lower_right->y = -1;
  list_add(pts, lower_right);
  return pts;
}

list_t *make_player(size_t num_points, size_t length, size_t width,
                    vector_t center) {
  list_t *player = list_init(num_points, free);
  if (length <= 1) {
    return make_pixel(center);
  }
  double x_pos, y_pos;
  double increment_angle = 2 * M_PI / num_points;
  double angle = 0;
  for (uint32_t i = 0; i < num_points; i++) {
    x_pos = cos(angle) * width + center.x;
    y_pos = sin(angle) * length + center.y;
    vector_t *point = malloc(sizeof(vector_t));
    point->x = x_pos;
    point->y = y_pos;
    list_add(player, point);
    angle += increment_angle;
  }
  return player;
}

list_t *make_invader(uint32_t num_points, size_t radius, vector_t center) {
  double curr_angle = 0;
  double total_angle = M_PI;
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
  vector_t *point = malloc(sizeof(vector_t));
  point->x = center.x;
  point->y = center.y - 0.5 * radius;
  list_add(pts, point);
  return pts;
}

list_t *make_bullet(size_t width, size_t length, vector_t center) {
  list_t *bullet = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){center.x - (1 * width), center.y - (1 * length)};
  list_add(bullet, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){center.x - (1 * width), center.y + (1 * length)};
  list_add(bullet, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){center.x + (1 * width), center.y + (1 * length)};
  list_add(bullet, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){center.x + (1 * width), center.y - (1 * length)};
  list_add(bullet, v);
  return bullet;
}

vector_t vector(double x, double y) { return (vector_t){.x = x, .y = y}; }

// emscripten: init
state_t *emscripten_init() {
  // init sdl
  sdl_init(MIN_POSITION, WINDOW);

  // init state
  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();

  // initialize players
  list_t *player_pts =
      make_player(PLAYER_RESOLUTION, PLAYER_LENGTH, PLAYER_WIDTH, PLAYER_SPAWN);
  char *player_info = malloc(sizeof(char) * PLAYER_INFO_LENGTH);
  player_info = "player\0";
  body_t *player = body_init_with_info(player_pts, PLAYER_MASS, PLAYER_COLOR,
                                       player_info, NULL);
  body_set_centroid(player, PLAYER_SPAWN);
  body_set_velocity(player, VEC_ZERO);
  scene_add_body(state->scene, player);

  // initialize invaders
  double invader_radius =
      ((ROW_SIZE - (NUM_INVADERS_PER_ROW * PADDING_X)) / NUM_INVADERS_PER_ROW) /
      2; // TODO: change to smthg else when we stop using circles

  vector_t spawn = (vector_t){MIN_POSITION.x + PADDING_X + invader_radius,
                              WINDOW.y - PADDING_Y - invader_radius};

  vector_t center = spawn;

  for (size_t i = 0; i < ROWS; i++) {
    for (size_t j = 0; j < NUM_INVADERS_PER_ROW; j++) {
      list_t *invader_pts = make_invader(50, invader_radius, center);
      char *invader_info = malloc(sizeof(char) * (INVADER_INFO_LENGTH));
      invader_info = "invader\0";
      body_t *invader = body_init_with_info(invader_pts, INVADER_MASS,
                                            INVADER_COLOR, invader_info, NULL);
      body_set_centroid(invader, center);
      body_set_velocity(invader, INVADER_VEL);
      scene_add_body(state->scene, invader);
      center.x += invader_radius * 2 + PADDING_X;
    }
    center.x = PADDING_X + invader_radius;
    center.y -= (PADDING_Y + (invader_radius * 2));
  }

  return state;
}

void key_press_handler(state_t *state, char key, key_event_type_t type,
                       double held_time) {
  body_t *player = scene_get_body(state->scene, 0);

  // shooting bullets
  if (type == 0 && state->bullet_present == false) {
    if (key == ' ') {
      list_t *bullet_pts =
          make_bullet(BULLET_WIDTH, BULLET_LENGTH, body_get_centroid(player));
      char *bullet_info = malloc(sizeof(char) * (PLAYER_BULLET_INFO_LENGTH));
      bullet_info = "player bullet\0";
      body_t *bullet = body_init_with_info(bullet_pts, BULLET_MASS,
                                           BULLET_COLOR, bullet_info, NULL);
      body_set_velocity(bullet, BULLET_VEL);
      scene_add_body(state->scene, bullet);

      for (size_t i = 0; i < scene_bodies(state->scene); i++) {
        if (!strcmp("invader\0",
                    body_get_info(scene_get_body(state->scene, i)))) {
          create_destructive_collision(state->scene, scene_get_body(state->scene, i), bullet);
        }
      }

      state->time_since_player_shot = 0;
      state->bullet_present = true;
    }
  }

  // player controls
  player = scene_get_body(state->scene, 0);
  if (type == 0) {
    switch (key) {
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
  // the following is to fix an issue where leaving a key while holding another
  // causes stopping.
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

void losing_states(state_t *state) {
  double invader_radius =
      ((ROW_SIZE - (NUM_INVADERS_PER_ROW * PADDING_X)) / NUM_INVADERS_PER_ROW) /
      2;
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    if (!strcmp("invader\0", body_get_info(scene_get_body(state->scene, i)))) {
      if ((body_get_centroid(scene_get_body(state->scene, i)).y -
           invader_radius) <= 0) {
        printf("%s\n", "invaded");
        exit(0);
      }
    }
    if (strcmp("player\0", body_get_info(scene_get_body(state->scene, 0)))) {
      printf("%s\n", "game over");
      exit(0);
    }
  }
}

// emscripten: main engine loop
void emscripten_main(state_t *state) {
  // sdl: clear window
  sdl_clear();

  // update state time_s
  double dt = time_since_last_tick();
  state->time_s += dt;
  state->time_since_player_shot += dt;
  state->time_since_invader_shot += dt;

  // handle keypresses
  sdl_on_key(key_press_handler);
  // while (!sdl_is_done(state));

  if (scene_bodies(state->scene) == 1) {
    printf("%s\n", "Well Played");
    exit(0);
  }

  // random bullet spawns
  if (state->time_since_invader_shot >= BULLET_COOLDOWN) {
    double shooting_invader_idx = rand_range(1, scene_bodies(state->scene));
    list_t *bullet_pts = make_bullet(
        BULLET_WIDTH, BULLET_LENGTH,
        body_get_centroid(scene_get_body(state->scene, shooting_invader_idx)));
    char *bullet_info = malloc(sizeof(char) * INVADER_BULLET_INFO_LENGTH);
    bullet_info = "invader bullet\0";
    body_t *bullet = body_init_with_info(bullet_pts, BULLET_MASS, BULLET_COLOR,
                                         bullet_info, NULL);
    body_t *player = scene_get_body(state->scene, 0);

    scene_add_body(state->scene, bullet);
    body_set_velocity(bullet, vec_negate(BULLET_VEL));
    state->time_since_invader_shot = 0;

    create_destructive_collision(state->scene, bullet, player);
  }

  // invaders coming closer to player when reaching end of screen
  double invader_radius =
      ((ROW_SIZE - (NUM_INVADERS_PER_ROW * PADDING_X)) / NUM_INVADERS_PER_ROW) /
      2;
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    if (!strcmp("invader\0", body_get_info(scene_get_body(state->scene, i)))) {
      body_t *invader = scene_get_body(state->scene, i);
      if (((body_get_centroid(invader).x + invader_radius + PADDING_X >
            WINDOW.x) &&
           (vec_equals(body_get_velocity(invader), INVADER_VEL))) ||
          ((body_get_centroid(invader).x - invader_radius - PADDING_X < 0) &&
           (vec_equals(body_get_velocity(invader), vec_negate(INVADER_VEL))))) {
        body_set_velocity(invader,
                          vec_negate(body_get_velocity(
                              invader))); // since y velocity is 0, this works
        double y_shift = -(2 * invader_radius + PADDING_Y) * ROWS;
        vector_t y_shift_vec = (vector_t){.x = 0, .y = y_shift};
        body_set_centroid(invader,
                          vec_add(body_get_centroid(invader), y_shift_vec));
      }
    }
  }

  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    char *player_bullet_info_comparison =
        malloc(sizeof(char) * PLAYER_BULLET_INFO_LENGTH);
    player_bullet_info_comparison = "player bullet\0";
    if (!strcmp(player_bullet_info_comparison,
                body_get_info(scene_get_body(state->scene, i)))) {
      if (body_get_centroid(scene_get_body(state->scene, i)).y -
              BULLET_LENGTH >=
          WINDOW.y) {
        scene_remove_body(state->scene, i);
        state->bullet_present = false;
      }
    }
  }

  body_t *player = scene_get_body(state->scene, 0);
  if (body_get_centroid(player).x > WINDOW.x) {
    vector_t new_centroid = body_get_centroid(player);
    new_centroid.x = 0;
    body_set_centroid(player, new_centroid);
  } else if (body_get_centroid(player).x < 0) {
    vector_t new_centroid = body_get_centroid(player);
    new_centroid.x = WINDOW.x;
    body_set_centroid(player, new_centroid);
  }

  // update scene & bodies
  scene_tick(state->scene, dt);

  bool found = false;
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    if (!strcmp("player bullet\0",
                body_get_info(scene_get_body(state->scene, i)))) {
      found = true;
    }
  }
  if (!found) {
    state->bullet_present = false;
  }

  // draws all the bodies in a scene
  scene_draw(state->scene);

  // checks if game is over
  losing_states(state);

  // sdl: show
  sdl_show();
}

// emscripten: free resources
void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
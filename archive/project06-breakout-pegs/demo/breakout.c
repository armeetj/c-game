
// includes
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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// state def
typedef struct state {
  scene_t *scene;
  bool game_started;
  double hack_cooldown;
  bool inverted_color;
} state_t;

// window constants
const vector_t WINDOW = (vector_t){.x = 672, .y = 768};
const vector_t CENTER = (vector_t){.x = 336, .y = 384};
const vector_t MIN_POSITION = (vector_t){.x = 0, .y = 0};

// wall constants
const double WALL_THICKNESS = 20;
const double NUM_WALL_POINTS = 9;
const size_t WALL_LEFT_INFO_LENGTH = 10;
const size_t WALL_RIGHT_INFO_LENGTH = 11;
const size_t WALL_TOP_INFO_LENGTH = 9;
const double WALL_MASS = INFINITY;
const size_t WALL_LEFT_BODIES_INDEX = 2;
const size_t WALL_TOP_BODIES_INDEX = 3;
const size_t WALL_RIGHT_BODIES_INDEX = 4;
const rgb_color_t WALL_COLOR = (rgb_color_t){.r = 0.3, .g = 0.3, .b = 0.3};

// ground constants
const double GROUND_HEIGHT = 6;
const size_t GROUND_INFO_LENGTH = 7;
const double GROUND_MASS = INFINITY;
const size_t GROUND_BODIES_INDEX = 5;
const rgb_color_t GROUND_COLOR = (rgb_color_t){.r = 0, .g = 0, .b = 0};

// player constants
const rgb_color_t PLAYER_COLOR = (rgb_color_t){.r = 1, .g = 1, .b = 1};
const rgb_color_t PLAYER_COLOR_HACKED = (rgb_color_t){.r = 0, .g = 1, .b = 1};
const double PLAYER_WIDTH = 100;
const double PLAYER_HEIGHT = 20;
const vector_t PLAYER_SPAWN = (vector_t){.x = 336, .y = 10};
const size_t PLAYER_INFO_LENGTH = 7;
const double PLAYER_MASS = INFINITY;
const double PLAYER_SPEED = 500;
const size_t PLAYER_BODIES_INDEX = 0;

// ball constants
const rgb_color_t BALL_COLOR = (rgb_color_t){.r = 1, .g = 0.1, .b = 0};
const double BALL_SIZE = 8;
const double BALL_MASS = 1;
const double BALL_RESOLUTION = 20;
const size_t BALL_BODIES_INDEX = 1;
const vector_t BALL_SPAWN = (vector_t){.x = 336, .y = 40};
const size_t BALL_INFO_LENGTH = 5;
const double BALL_MIN_SPEED = 50;
const double BALL_MAX_SPEED = 250;

// brick constants
const double NUM_BRICKS_PER_ROW = 10;
const double ROWS = 3;
const double BRICK_HEIGHT = 20;
const double BRICK_MASS = INFINITY;
const rgb_color_t BRICK_COLOR = (rgb_color_t){.r = 1, .g = 1, .b = 1};
const size_t BRICK_INFO_LENGTH = 6;
const double PADDING = 100;

// hack constants
const double HACK_APS = 40; // per second limit

// helper functions
double rand_range(double low, double high) {
  return ((double)rand() * (high - low)) / (double)RAND_MAX + low;
}

// credit:
// https://justinparrtech.com/JustinParr-Tech/spectrum-generating-color-function-using-sine-waves/
rgb_color_t rainbow_color(double n, double m) {
  double a = ((5 * M_PI * n) / (3 * m)) + (M_PI / 2);
  double y = sin(a) * 192 + 128;
  double r = fmax(0, fmin(255, y));
  y = sin(a - 2 * M_PI / 3) * 192 + 128;
  double g = fmax(0, fmin(255, y));
  y = sin(a - 4 * M_PI / 3) * 192 + 128;
  double b = fmax(0, fmin(255, y));
  return (rgb_color_t){.r = r / 255, .g = g / 255, .b = b / 255};
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

list_t *make_rectangle(double width, double height, vector_t center) {
  list_t *rectangle = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){-0.5 * width + center.x, -0.5 * height + center.y};
  list_add(rectangle, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+0.5 * width + center.x, -0.5 * height + center.y};
  list_add(rectangle, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+0.5 * width + center.x, +0.5 * height + center.y};
  list_add(rectangle, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){-0.5 * width + center.x, +0.5 * height + center.y};
  list_add(rectangle, v);
  return rectangle;
}

list_t *make_left_wall() {
  return make_rectangle(WALL_THICKNESS, WINDOW.y,
                        (vector_t){0.5 * WALL_THICKNESS, CENTER.y});
}

list_t *make_top_wall() {
  return make_rectangle(WINDOW.x, WALL_THICKNESS,
                        (vector_t){CENTER.x, WINDOW.y - 0.5 * WALL_THICKNESS});
}

list_t *make_right_wall() {
  return make_rectangle(WALL_THICKNESS, WINDOW.y,
                        (vector_t){WINDOW.x - 0.5 * WALL_THICKNESS, CENTER.y});
}

void default_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                               void *aux) {
  bool *impulsed_last_tick = (bool *)aux;

  list_t *shape1 = body_get_shape(body1);
  list_t *shape2 = body_get_shape(body2);

  if (find_collision(shape1, shape2).collided) {
    if (!(*impulsed_last_tick)) {
      *impulsed_last_tick = true;
      body_add_elastic_impulse(body1, body2, 1.0); // TODO: MAGIC NUMBER
    }
  } else {
    *impulsed_last_tick = false;
  }
}

void game_reset(state_t *);

void ground_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                              void *aux) {
  state_t *state = (state_t *)aux;

  list_t *shape1 = body_get_shape(body1);
  list_t *shape2 = body_get_shape(body2);

  if (find_collision(shape1, shape2).collided) {
    game_reset(state);
  }
}

void brick_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                             void *aux) {
  state_t *state = (state_t *)aux;

  list_t *shape1 = body_get_shape(body1);
  list_t *shape2 = body_get_shape(body2);

  if (find_collision(shape1, shape2).collided) {
    body_add_elastic_impulse(body1, body2, 1.0);
    body_remove(body2);
  }
}

void test_game_win(state_t *state) {
  if (scene_bodies(state->scene) == 6) {
    printf("Game won!\n");
    exit(0);
  }
}

void game_init(state_t *state) {
  state->scene = scene_init();
  state->game_started = false;
  state->inverted_color = false;
  state->hack_cooldown = 0;

  // initialize player
  list_t *player_pts =
      make_rectangle(PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_SPAWN);
  char *player_info = malloc(sizeof(char) * PLAYER_INFO_LENGTH);
  player_info = "player\0";
  body_t *player = body_init_with_info(player_pts, PLAYER_MASS, PLAYER_COLOR,
                                       player_info, NULL);
  body_set_centroid(player, PLAYER_SPAWN);
  body_set_velocity(player, VEC_ZERO);
  scene_add_body(state->scene, player);

  // initialize ball
  list_t *ball_pts = make_circle(BALL_RESOLUTION, BALL_SIZE, BALL_SPAWN);
  char *ball_info = malloc(sizeof(char) * BALL_INFO_LENGTH);
  ball_info = "ball\0";
  body_t *ball =
      body_init_with_info(ball_pts, BALL_MASS, BALL_COLOR, ball_info, NULL);
  body_set_centroid(ball, BALL_SPAWN);
  body_set_velocity(ball, VEC_ZERO);
  scene_add_body(state->scene, ball);

  // initialize wall
  list_t *wall_left_pts = make_left_wall();
  char *wall_left_info = malloc(sizeof(char) * WALL_LEFT_INFO_LENGTH);
  wall_left_info = "wall_left\0";
  list_t *wall_top_pts = make_top_wall();
  char *wall_top_info = malloc(sizeof(char) * WALL_TOP_INFO_LENGTH);
  wall_top_info = "wall_top\0";
  list_t *wall_right_pts = make_right_wall();
  char *wall_right_info = malloc(sizeof(char) * WALL_RIGHT_INFO_LENGTH);
  wall_right_info = "wall_right\0";
  body_t *wall_left =
      body_init_with_info(wall_left_pts, WALL_MASS, WALL_COLOR, wall_left_info,
                          NULL); // TODO: Check freer
  body_t *wall_top =
      body_init_with_info(wall_top_pts, WALL_MASS, WALL_COLOR, wall_top_info,
                          NULL); // TODO: Check freer
  body_t *wall_right = body_init_with_info(wall_right_pts, WALL_MASS,
                                           WALL_COLOR, wall_right_info,
                                           NULL); // TODO: Check freer

  scene_add_body(state->scene, wall_left);
  scene_add_body(state->scene, wall_top);
  scene_add_body(state->scene, wall_right);

  // initialize ground
  vector_t ground_center =
      (vector_t){.5 * (WINDOW.x - MIN_POSITION.x), (-.5 * GROUND_HEIGHT)};
  list_t *ground_pts =
      make_rectangle(WINDOW.x - MIN_POSITION.x, GROUND_HEIGHT, ground_center);
  char *ground_info = malloc(sizeof(char) * GROUND_INFO_LENGTH);
  ground_info = "ground\0";
  body_t *ground = body_init_with_info(ground_pts, GROUND_MASS, GROUND_COLOR,
                                       ground_info, NULL);
  scene_add_body(state->scene, ground);

  // initialize bricks
  double brick_width = (WINDOW.x - 2 * WALL_THICKNESS) / (NUM_BRICKS_PER_ROW);

  vector_t brick_spawn =
      (vector_t){MIN_POSITION.x + WALL_THICKNESS + .5 * brick_width,
                 WINDOW.y - WALL_THICKNESS - 0.5 * BRICK_HEIGHT - PADDING};

  vector_t center = brick_spawn;

  for (size_t i = 0; i < ROWS; i++) {
    for (size_t j = 0; j < NUM_BRICKS_PER_ROW; j++) {
      list_t *brick_pts = make_rectangle(brick_width, BRICK_HEIGHT, center);
      char *brick_info = malloc(sizeof(char) * (BRICK_INFO_LENGTH));
      brick_info = "brick\0";
      body_t *brick = body_init_with_info(brick_pts, BRICK_MASS,
                                          rainbow_color(j, NUM_BRICKS_PER_ROW),
                                          brick_info, NULL);
      body_set_centroid(brick, center);
      scene_add_body(state->scene, brick);
      create_collision(state->scene, ball, brick, brick_collision_handler, NULL,
                       NULL);
      center.x += brick_width;
    }
    center.x = MIN_POSITION.x + WALL_THICKNESS + .5 * brick_width;
    center.y -= BRICK_HEIGHT;
  }

  // initialize collision: balls <--> walls
  bool *impulse_status_wall_left = malloc(sizeof(bool));
  *impulse_status_wall_left = false;
  bool *impulse_status_wall_top = malloc(sizeof(bool));
  *impulse_status_wall_top = false;
  bool *impulse_status_wall_right = malloc(sizeof(bool));
  *impulse_status_wall_right = false;
  create_collision(state->scene, ball, wall_left, default_collision_handler,
                   impulse_status_wall_left, free);
  create_collision(state->scene, ball, wall_top, default_collision_handler,
                   impulse_status_wall_top, free);
  create_collision(state->scene, ball, wall_right, default_collision_handler,
                   impulse_status_wall_right, free);

  // initialize collision: ball <--> ground
  create_collision(state->scene, ball, ground, ground_collision_handler, state,
                   NULL);

  bool *impulse_status_paddle = malloc(sizeof(bool));
  *impulse_status_paddle = false;
  // initialize collision: ball <--> paddle
  create_collision(state->scene, ball, player, default_collision_handler,
                   impulse_status_paddle, free);
}

void game_reset(state_t *state) {
  game_init(state);
  // printf("game reset\n");
}

// emscripten: init
state_t *emscripten_init() {
  printf("Hold <h> to toggle **HACK MODE**\n");
  printf("Hold <j> to toggle **EMPOWERED HACK MODE**\n");

  // init sdl
  sdl_init(MIN_POSITION, WINDOW);

  // init state
  state_t *state = malloc(sizeof(state_t));

  game_init(state);

  return state;
}

// keyboard events
void key_press_handler(state_t *state, char key, key_event_type_t type,
                       double held_time) {
  body_t *player = scene_get_body(state->scene, 0);
  body_t *ball = scene_get_body(state->scene, 1);
  if (!state->game_started) {
    state->game_started = true;
    // body_set_velocity(ball, BALL_INIT_VEL);
    body_set_velocity(
        ball, (vector_t){.x = rand_range(BALL_MIN_SPEED, BALL_MAX_SPEED) *
                              pow(-1, round(rand_range(1, 2))),
                         .y = rand_range(BALL_MIN_SPEED, BALL_MAX_SPEED)});
  }
  if (type == 0) {
    switch (key) {
    case 3:
    case 'd':
      body_set_velocity(player, vector(PLAYER_SPEED, 0));
      break;
    case 1:
    case 'a':
      body_set_velocity(player, vector(-PLAYER_SPEED,
                                       0)); // works because y-velocity = 0
      break;
    }
  }
  // the following is to fix an issue where leaving a key while holding another
  // causes stopping.
  else if (type == 1 &&
           vec_equals(body_get_velocity(player), vector(PLAYER_SPEED, 0))) {
    switch (key) {
    case 3:
    case 'd':
      body_set_velocity(player, VEC_ZERO);
      break;
    }
  } else if (type == 1 &&
             vec_equals(body_get_velocity(player), vector(-PLAYER_SPEED, 0))) {
    switch (key) {
    case 1:
    case 'a':
      body_set_velocity(player, VEC_ZERO);
      break;
    }
  }

  // hack mode
  if (type == 0) {
    if (key == 'h') {
      if (!state->inverted_color) {
        for (size_t i = 1; i < scene_bodies(state->scene); i++) {
          body_set_color(
              scene_get_body(state->scene, i),
              color_inverted(body_get_color(scene_get_body(state->scene, i))));
        }
        state->inverted_color = true;
      }
      if (state->hack_cooldown > (1 / HACK_APS)) {
        double ball_center = body_get_centroid(ball).x;
        double player_center = body_get_centroid(player).x;

        if (player_center > ball_center) {
          body_set_velocity(player, vector(-PLAYER_SPEED, 0));
        } else if (player_center < ball_center) {
          body_set_velocity(player, vector(PLAYER_SPEED, 0));
        }
        state->hack_cooldown = 0;
      }
    } else if (key == 'j') {
      body_set_centroid(player, vector(body_get_centroid(ball).x,
                                       body_get_centroid(player).y));
      body_set_velocity(player, VEC_ZERO);
      if (!state->inverted_color) {
        for (size_t i = 1; i < scene_bodies(state->scene); i++) {
          body_set_color(
              scene_get_body(state->scene, i),
              color_inverted(body_get_color(scene_get_body(state->scene, i))));
        }
        state->inverted_color = true;
      }
      state->hack_cooldown = 0;
    }
  }
  if (type == 1) {
    if (key == 'h') {
      for (size_t i = 1; i < scene_bodies(state->scene); i++) {
        body_set_color(
            scene_get_body(state->scene, i),
            color_inverted(body_get_color(scene_get_body(state->scene, i))));
      }
      state->inverted_color = false;
      body_set_velocity(player, VEC_ZERO);
    } else if (key == 'j') {
      for (size_t i = 1; i < scene_bodies(state->scene); i++) {
        body_set_color(
            scene_get_body(state->scene, i),
            color_inverted(body_get_color(scene_get_body(state->scene, i))));
      }
      state->inverted_color = false;
    }
  }
}

// emscripten: main loop
void emscripten_main(state_t *state) {

  // sdl: clear window
  sdl_clear();

  double dt = time_since_last_tick();
  state->hack_cooldown += dt;

  // handle keypresses
  sdl_on_key(key_press_handler);

  body_t *player = scene_get_body(state->scene, 0);
  if ((body_get_centroid(player).x + (.5 * PLAYER_WIDTH) + WALL_THICKNESS >
       WINDOW.x) &&
      vec_equals(body_get_velocity(player), vector(PLAYER_SPEED, 0))) {
    body_set_velocity(player, VEC_ZERO);
  }
  if ((body_get_centroid(player).x - (.5 * PLAYER_WIDTH) - WALL_THICKNESS <
       MIN_POSITION.x) &&
      vec_equals(body_get_velocity(player), vector(-PLAYER_SPEED, 0))) {
    body_set_velocity(player, VEC_ZERO);
  }

  // update scene & bodies
  scene_tick(state->scene, dt);

  test_game_win(state);

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
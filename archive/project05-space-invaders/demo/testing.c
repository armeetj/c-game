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

// // invader constants
// const double PADDING_X = 15;
// const double PADDING_Y = 10;
// const double NUM_INVADERS_PER_ROW = 1;
// const double ROWS = 1;
// const double ROW_SIZE = 600;
// const vector_t INVADER_VEL = (vector_t){.x = 60, .y = 0};
// const double INVADER_MASS = 1;
// const double INVADER_RESOLUTION = 100;
// const rgb_color_t INVADER_COLOR = (rgb_color_t){.r = 1, .g = 1, .b = 1};
// // const double SPAWN = (vector_t) {MIN_POSITION.x + PADDING_X, WINDOW.y -
// // PADDING_Y} // TODO: remove if handled in init

// // bullet constants
// const double BULLET_WIDTH = 3;
// const double BULLET_LENGTH = 5;
// const double BULLET_MASS = 1;
// const rgb_color_t BULLET_COLOR = (rgb_color_t){.r = 1, .g = 1, .b = 1};
// const vector_t BULLET_VEL = (vector_t){.x = 0, .y = 200};
// const double BULLET_COOLDOWN = 1;

// // player constants
// const double PLAYER_RADIUS = 20;
// const vector_t PLAYER_MOVE_SPEED = (vector_t){.x = 120, .y = 0};
// const double PLAYER_RESOLUTION = 100;
// const double PLAYER_PADDING = 20;
// const vector_t PLAYER_SPAWN = (vector_t){384, 30};
// const double PLAYER_MASS = 1;
// const rgb_color_t PLAYER_COLOR = (rgb_color_t){.r = 0, .g = 0, .b = 1};

// actual constants
const double M = 10;
const double A = 900;
const double DT = 1e-6;
const int STEPS = 1000000;

// helper functions
double rand_range(double low, double high) {
  return ((double)rand() * (high - low)) / (double)RAND_MAX + low;
}

/**
 * list_t *make_player(vector_t center) {
    list_t *pits = lis
  }
*/

// list_t *make_pixel(vector_t position) {
//   list_t *pts = list_init(4, free);
//   vector_t *lower_left = malloc(sizeof(vector_t));
//   lower_left->x = position.x - 1;
//   lower_left->y = position.y - 1;
//   list_add(pts, lower_left);

//   vector_t *upper_left = malloc(sizeof(vector_t));
//   upper_left->x = position.x - 1;
//   upper_left->y = position.y + 1;
//   list_add(pts, upper_left);

//   vector_t *upper_right = malloc(sizeof(vector_t));
//   upper_right->x = position.x + 1;
//   upper_right->y = position.y + 1;
//   list_add(pts, upper_right);

//   vector_t *lower_right = malloc(sizeof(vector_t));
//   lower_right->x = position.x + 1;
//   lower_right->y = position.y - 1;
//   list_add(pts, lower_right);
//   return pts;
// }

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

// list_t *make_circle(size_t num_points, size_t length, vector_t center) {
//   list_t *circle = list_init(num_points, free);
//   if (length <= 1) {
//     return make_pixel(center);
//   }
//   double x_pos, y_pos;
//   double increment_angle = 2 * M_PI / num_points;
//   double angle = 0;
//   for (uint32_t i = 0; i < num_points; i++) {
//     x_pos = cos(angle) * length + center.x;
//     y_pos = sin(angle) * length + center.y;
//     vector_t *point = malloc(sizeof(vector_t));
//     point->x = x_pos;
//     point->y = y_pos;
//     list_add(circle, point);
//     angle += increment_angle;
//   }
//   return circle;
// }

// list_t *make_bullet(size_t width, size_t length, vector_t center) {
//   list_t *bullet = list_init(4, free);
//   vector_t *v = malloc(sizeof(*v));
//   *v = (vector_t){center.x - (1 * width), center.y - (1 * length)};
//   list_add(bullet, v);
//   v = malloc(sizeof(*v));
//   *v = (vector_t){center.x - (1 * width), center.y + (1 * length)};
//   list_add(bullet, v);
//   v = malloc(sizeof(*v));
//   *v = (vector_t){center.x + (1 * width), center.y + (1 * length)};
//   list_add(bullet, v);
//   v = malloc(sizeof(*v));
//   *v = (vector_t){center.x + (1 * width), center.y - (1 * length)};
//   list_add(bullet, v);
//   return bullet;
// }

vector_t vector(double x, double y) { return (vector_t){.x = x, .y = y}; }

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

  // update state time_s
  double dt = time_since_last_tick();
  // state->time_s += dt;
  // state->time_since_player_shot += dt;

  // handle keypresses
  sdl_on_key(key_press_handler);
  // while (!sdl_is_done(state));

  // // invaders coming closer to player when reaching end of screen
  // double invader_radius = ((ROW_SIZE - (NUM_INVADERS_PER_ROW * PADDING_X)) /
  // NUM_INVADERS_PER_ROW) / 2; for (size_t i = 1; i < 1 + NUM_INVADERS_PER_ROW
  // * ROWS - state->num_invaders_shot; i++) {
  //   body_t *invader = scene_get_body(state->scene, i);
  //   if (((body_get_centroid(invader).x + invader_radius + PADDING_X >
  //   WINDOW.x) &&
  //        (vec_equals(body_get_velocity(invader), INVADER_VEL))) ||
  //       ((body_get_centroid(invader).x - invader_radius - PADDING_X < 0) &&
  //        (vec_equals(body_get_velocity(invader), vec_negate(INVADER_VEL)))))
  //        {
  //     body_set_velocity(invader, vec_negate(body_get_velocity(invader))); //
  //     since y velocity is 0, this works double y_shift = -(2 * invader_radius
  //     + PADDING_Y) * 3; vector_t y_shift_vec = (vector_t){.x = 0, .y =
  //     y_shift}; body_set_centroid(invader,
  //                       vec_add(body_get_centroid(invader), y_shift_vec));
  //   }
  // }

  // update scene & bodies
  scene_tick(state->scene, DT * 1e4);

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
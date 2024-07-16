#include "body.h"
#include "text.h"
#include "collision.h"
#include "color.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include "player.h"
#include "utils.h"
// #include <SDL2/SDL_mixer.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// physics constants
#define SPRING_CONST 8000.0
#define DRAG_CONST 1000.0

// window constants
const vector_t WINDOW = (vector_t){.x = 1600, .y = 900};
const vector_t CENTER = (vector_t){.x = 800, .y = 450};
const vector_t MIN_POSITION = (vector_t){.x = 0, .y = 0};

// wall constants
const double WALL_THICKNESS = 10;
const double WALL_MASS = INFINITY;
const size_t WALL_LEFT_BODIES_INDEX = 2;
const size_t WALL_TOP_BODIES_INDEX = 3;
const size_t WALL_RIGHT_BODIES_INDEX = 4;
const double WALL_IMPULSE = 80000;
const color_t WALL_COLOR = (color_t){.r = 0.3, .g = 0.3, .b = 0.3, .a = 1};

// game constants
const size_t GAME_NUM_PLAYERS = 4;
const double PLAYER_COLLISION_IMPULSE = 100;
const size_t INFO_MAX_LEN = 100;
const double dt = 0.01;

// color constants
const color_t COLOR_WHITE = (color_t) {1, 1, 1, 1};

const color_t COLOR_PLAYER1_CHOICE1 = (color_t){255 / 255.0, 147 / 255.0, 140 / 255.0, 1};
const color_t COLOR_PLAYER1_CHOICE2 = (color_t){234 / 255.0, 210 / 255.0, 172 / 255.0, 1};
const color_t COLOR_PLAYER1_CHOICE3 = (color_t){156 / 255.0, 175 / 255.0, 183 / 255.0, 1};
const color_t COLOR_PLAYER1_CHOICE4 = (color_t){181 / 255.0, 217 / 255.0, 156 / 255.0, 1};

const color_t COLOR_PLAYER2_CHOICE1 = (color_t){128 / 255.0, 35 / 255.0, 146 / 255.0, 1};
const color_t COLOR_PLAYER2_CHOICE2 = (color_t){165 / 255.0, 248 / 255.0, 211 / 255.0, 1};
const color_t COLOR_PLAYER2_CHOICE3 = (color_t){99 / 255.0, 210 / 255.0, 255.0 / 255.0, 1};
const color_t COLOR_PLAYER2_CHOICE4 = (color_t){249 / 255.0, 0 / 255.0, 147 / 255.0, 1};

const color_t COLOR_PLAYER3_CHOICE1 = (color_t){15 / 255.0, 29 / 255.0, 115 / 255.0, 1};
const color_t COLOR_PLAYER3_CHOICE2 = (color_t){44 / 255.0, 103 / 255.0, 186 / 255.0, 1};
const color_t COLOR_PLAYER3_CHOICE3 = (color_t){75 / 255.0, 166 / 255.0, 179 / 255.0, 1};
const color_t COLOR_PLAYER3_CHOICE4 = (color_t){106 / 255.0, 221 / 255.0, 128 / 255.0, 1};

const color_t COLOR_PLAYER4_CHOICE1 = (color_t){232 / 255.0, 223 / 255.0, 24 / 255.0, 1};
const color_t COLOR_PLAYER4_CHOICE2 = (color_t){252 / 255.0, 225 / 255.0, 228 / 255.0, 1};
const color_t COLOR_PLAYER4_CHOICE3 = (color_t){165 / 255.0, 213 / 255.0, 213 / 255.0, 1};
const color_t COLOR_PLAYER4_CHOICE4 = (color_t){176 / 255.0, 203 / 255.0, 246 / 255.0, 1};

const color_t COLOR_BLACK = (color_t){0, 0, 0, 1};
const color_t ERROR_COLOR = (color_t){1, 0, 0, 1};
const color_t SLYCE_COLOR = (color_t){.r = 52 / 255.0, .g = 235 / 255.0, .b = 189 / 255.0, .a = 1};

// main menu constants
const double TITLE_HEIGHT = 150;
const double TITLE_WIDTH = 75;
const double START_HEIGHT = 60;
const double START_WIDTH = 30;
const vector_t PLAYER_1_SETTINGS_CENTER = (vector_t){300, 700};
const vector_t PLAYER_2_SETTINGS_CENTER = (vector_t){1300, 700};
const vector_t PLAYER_3_SETTINGS_CENTER = (vector_t){300, 100};
const vector_t PLAYER_4_SETTINGS_CENTER = (vector_t){1300, 100};
const double PLAYER_TEXT_HEIGHT = 40;
const double PLAYER_TEXT_WIDTH = 20;
const double ERROR_TEXT_HEIGHT = 50;
const double ERROR_TEXT_WIDTH = 25;
const double ERROR_DURATION = 5;
const double HELP_HEIGHT = 30;
const double HELP_WIDTH = 15;
const color_t HELP_COLOR = (color_t){1, 1, 1, 0.4};
const double HELP_OPACITY = 0.4;

// choices constants
const double CHOICE_SIZE = 50;
const double CHOICE_MASS = 1;
const vector_t CHOICE_SPAWN_POSITION = (vector_t){0, -100};

// food constants
const double FOOD_MIN_SIZE_FACTOR = 0.5;
const double FOOD_MAX_SIZE_FACTOR = 1;
const size_t FOOD_COUNT_INITIAL = 13;
const double FOOD_SIDE_LENGTH = 10;
const double FOOD_SPAWN_TIME = 5;
const time_t FOOD_FLASH_FREQUENCY = 1;
const color_t FOOD_COLOR = (color_t){.r = 1, .g = 0.72, .b = 0.69, .a = 1};
const size_t PTS_IN_PELLET = 4;

// sound constants
const int FREE_CHANNEL = -1;
const int SECONDARY_CHANNEL = 0;
const int REPEATED_SOUND_CHANNEL = 1;

// state def
typedef struct state
{
  scene_t *scene_game;
  scene_t *scene_menu;
  text_t *timer;
  bool sound_playing;
  list_t *players;
  bool game_started;
  bool left_movement;
  bool right_movement;
  double time_since_dash;
  double game_time;
  double time_since_pellet_spawn;
} state_t;

list_t *make_left_wall()
{
  return make_rectangle(WALL_THICKNESS, WINDOW.y,
                        (vector_t){-0.5 * WALL_THICKNESS, CENTER.y});
}

list_t *make_bottom_wall()
{
  return make_rectangle(WINDOW.x, WALL_THICKNESS,
                        (vector_t){CENTER.x, -0.5 * WALL_THICKNESS});
}

list_t *make_top_wall()
{
  return make_rectangle(WINDOW.x, WALL_THICKNESS,
                        (vector_t){CENTER.x, WINDOW.y + 0.5 * WALL_THICKNESS});
}

list_t *make_right_wall()
{
  return make_rectangle(WALL_THICKNESS, WINDOW.y,
                        (vector_t){WINDOW.x + 0.5 * WALL_THICKNESS, CENTER.y});
}

void wall_collision_handler(body_t *body1, body_t *body2, vector_t axis, void *aux)
{
  list_t *shape1 = body_get_shape(body1);
  list_t *shape2 = body_get_shape(body2);
  if (find_collision(shape1, shape2).collided)
  {
    char *info = malloc(sizeof(char) * INFO_MAX_LEN);
    strcpy(info, list_get((list_t *)body_get_info(body2), 0));
    if (strcmp(info, "wall_top") == 0)
    {
      vector_t velocity = (vector_t){.x = 0, .y = -WALL_IMPULSE};
      body_add_impulse(body1, velocity);
    }
    else if (strcmp(info, "wall_bottom") == 0)
    {
      vector_t velocity = (vector_t){.x = 0, .y = WALL_IMPULSE};
      body_add_impulse(body1, velocity);
    }
    else if (strcmp(info, "wall_left") == 0)
    {
      vector_t velocity = (vector_t){.x = WALL_IMPULSE, .y = 0};
      body_add_impulse(body1, velocity);
    }
    else if (strcmp(info, "wall_right") == 0)
    {
      vector_t velocity = (vector_t){.x = -WALL_IMPULSE, .y = 0};
      body_add_impulse(body1, velocity);
    }
  }
  list_free(shape1);
  list_free(shape2);
}

void player_collision_handler(body_t *body1, body_t *body2, vector_t axis, void *aux)
{
  state_t *state = (state_t *)aux;
  if (list_size(state->players) > 0)
  {
    // assume body1 is the slug head
    // and body2 is a metabody from slug 2
    size_t player_id1 = *((size_t *)list_get((list_t *)body_get_info(body1), 1));
    size_t player_id2 = *((size_t *)list_get((list_t *)body_get_info(body2), 1));
    player_t *p1 = list_get(state->players, player_id1);
    player_t *p2 = list_get(state->players, player_id2);
    list_t *body1_pts = body_get_shape(body1);
    list_t *body2_pts = body_get_shape(body2);
    if (find_collision(body1_pts, body2_pts).collided && p1->cd_collide_player <= 0 && p2->cd_collide_player <= 0)
    {
      vector_t head_velocity = body_get_velocity(body1);
      vector_t body_velocity = body_get_velocity(body2);

      vector_t impulse_on_head = vec_multiply(PLAYER_COLLISION_IMPULSE, vec_perpendicular(body_velocity));
      if (fabs(vec_angle(impulse_on_head, head_velocity)) < M_PI / 2)
      {
        impulse_on_head = vec_multiply(-1, impulse_on_head);
      }
      vector_t impulse_on_body = vec_multiply(PLAYER_COLLISION_IMPULSE, head_velocity);

      body_add_impulse(body1, impulse_on_head);
      body_add_impulse(body2, impulse_on_body);
      player_refresh_cd_collide_player(p1);
      player_refresh_cd_collide_player(p2);
      sdl_play_sound(FREE_CHANNEL, "assets/collide.wav", 0);
    }
    list_free(body1_pts);
    list_free(body2_pts);
  }
}

void bullet_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                              void *aux)
{
  state_t *state = (state_t *)aux;
  size_t bullet_player_id = *((size_t *)list_get((list_t *)body_get_info(body1), 1));
  char *body_impacted_type = list_get((list_t *)body_get_info(body2), 0);
  list_t *bullet_pts = body_get_shape(body1);
  list_t *impact_pts = body_get_shape(body2);

  if (find_collision(bullet_pts, impact_pts).collided)
  {
    if (strcmp(body_impacted_type, "wall_top") == 0 || strcmp(body_impacted_type, "wall_bottom") == 0 ||
        strcmp(body_impacted_type, "wall_left") == 0 || strcmp(body_impacted_type, "wall_right") == 0)
    {
      body_remove(body1);
    }
    else // if bullet hits a player
    {
      player_t *player_who_shot_bullet = list_get(state->players, bullet_player_id);
      size_t player_hit_id = *((size_t *)list_get((list_t *)body_get_info(body2), 1)); // hits player
      player_t *player_to_remove = list_get(state->players, player_hit_id);
      player_hit(player_who_shot_bullet, player_to_remove, body2, state->scene_game);
      body_remove(body1);
    }
  }
  list_free(bullet_pts);
  list_free(impact_pts);
}

void pellet_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                              void *aux)
{
  list_t *aux_casted = (list_t *)aux;
  state_t *state = list_get(aux_casted, 0);
  player_t *player = list_get(aux_casted, 1);
  list_t *snake_pts = body_get_shape(body1);
  list_t *food_pts = body_get_shape(body2);

  if (find_collision(snake_pts, food_pts).collided)
  {
    player_eat(player, body2, state->scene_game);
    body_t *added_body = player_add_body(player);
    scene_add_body(state->scene_game, added_body);
    create_drag(state->scene_game, DRAG_CONST, list_get(player->meta_bodies, list_size(player->meta_bodies) - 1));
    create_spring(state->scene_game, SPRING_CONST, list_get(player->meta_bodies, list_size(player->meta_bodies) - 1), list_get(player->meta_bodies, list_size(player->meta_bodies) - 2));
    for (size_t i = 0; i < GAME_NUM_PLAYERS; i++)
    { // add collisions between added body and other player heads
      if (list_get(state->players, i) != player)
      {
        body_t *player_head = player_get_head(list_get(state->players, i));
        create_collision(state->scene_game, player_head, added_body, player_collision_handler, state, NULL);
      }
    }
    for (size_t i = 0; i < scene_bodies(state->scene_game); i++) // add collisions between added body and existing bullets
    {
      body_t *curr_body = scene_get_body(state->scene_game, i);
      char *body_type = list_get((list_t *)body_get_info(curr_body), 0);
      if (strcmp(body_type, "bullet") == 0) // if its a bullet
      {
        size_t curr_body_player_id = *((size_t *)list_get((list_t *)body_get_info(curr_body), 1));
        if (curr_body_player_id != player->player_id)
        {
          create_collision(state->scene_game, curr_body, added_body, bullet_collision_handler, state, NULL);
        }
      }
    }
    body_remove(body2);
  }
  list_free(snake_pts);
  list_free(food_pts);
}

list_t *get_pu_types()
{
  list_t *pu_types = list_init(4, free);
  list_add(pu_types, "pu_base_speed");
  list_add(pu_types, "pu_bullet_speed");
  list_add(pu_types, "pu_rotate_rate");
  list_add(pu_types, "pu_dash_boost");
  return pu_types;
}

list_t *get_pu_colors()
{
  list_t *pu_colors = list_init(4, free);
  color_t *color1 = malloc(sizeof(color_t));
  color_t *color2 = malloc(sizeof(color_t));
  color_t *color3 = malloc(sizeof(color_t));
  color_t *color4 = malloc(sizeof(color_t));
  *color1 = (color_t){.r = 0.3, .b = 1, .g = 1, .a = 1};
  *color2 = (color_t){.r = 0.5, .b = 1, .g = 0.1, .a = 1};
  *color3 = (color_t){.r = 1, .b = 0.9, .g = 0, .a = 1};
  *color4 = (color_t){.r = 1, .b = 0, .g = 0.9, .a = 1};
  list_add(pu_colors, color1);
  list_add(pu_colors, color2);
  list_add(pu_colors, color3);
  list_add(pu_colors, color4);
  return pu_colors;
}

void spawn_pellet(state_t *state, list_t *pu_types, list_t *pu_colors)
{
  vector_t pellet_pos = (vector_t){.x = (double)rand_range(MIN_POSITION.x, WINDOW.x), .y = (double)rand_range(MIN_POSITION.y, WINDOW.y)};
  size_t pellet_type = (size_t)(rand_range(0, 1) * 4);
  list_t *info = list_init(2, free);
  char *body_type = malloc(sizeof(char) * INFO_MAX_LEN);
  char *pu_type = malloc(sizeof(char) * INFO_MAX_LEN);
  strcpy(body_type, "food\0");
  strcpy(pu_type, (char *)list_get(pu_types, pellet_type));
  list_add(info, body_type);
  list_add(info, pu_type);
  body_t *food = body_init_with_info(make_circle(6, FOOD_SIDE_LENGTH, pellet_pos), 1, *((color_t *)list_get(pu_colors, pellet_type)), info, list_free);
  body_set_glow(food, true);
  body_set_glow_radius(food, FOOD_SIDE_LENGTH);
  scene_add_body(state->scene_game, food);
  for (size_t k = 0; k < GAME_NUM_PLAYERS; k++)
  {
    list_t *aux = list_init(2, NULL);
    list_add(aux, state);
    list_add(aux, list_get(state->players, k));
    create_collision(state->scene_game, player_get_head(list_get(state->players, k)), food, pellet_collision_handler, aux, NULL);
  }
}

void spawn_color_choices(state_t *state, size_t *player_id, vector_t center, color_t c1, color_t c2, color_t c3, color_t c4)
{
  vector_t choice1_pos = vec_add(center, (vector_t){-30, 30});
  vector_t choice2_pos = vec_add(center, (vector_t){30, 30});
  vector_t choice3_pos = vec_add(center, (vector_t){-30, -30});
  vector_t choice4_pos = vec_add(center, (vector_t){30, -30});

  body_t *choice1 = body_init_with_info(make_rectangle(CHOICE_SIZE, CHOICE_SIZE, choice1_pos), CHOICE_MASS, c1, player_id, free);
  body_t *choice2 = body_init_with_info(make_rectangle(CHOICE_SIZE, CHOICE_SIZE, choice2_pos), CHOICE_MASS, c2, player_id, free);
  body_t *choice3 = body_init_with_info(make_rectangle(CHOICE_SIZE, CHOICE_SIZE, choice3_pos), CHOICE_MASS, c3, player_id, free);
  body_t *choice4 = body_init_with_info(make_rectangle(CHOICE_SIZE, CHOICE_SIZE, choice4_pos), CHOICE_MASS, c4, player_id, free);

  scene_add_body(state->scene_menu, choice1);
  scene_add_body(state->scene_menu, choice2);
  scene_add_body(state->scene_menu, choice3);
  scene_add_body(state->scene_menu, choice4);
}

void game_init(state_t *state)
{
  state->scene_game = scene_init();
  state->game_started = true;

  for (size_t i = 0; i < GAME_NUM_PLAYERS; i++)
  {
    player_t *player = list_get(state->players, i);
    for (size_t j = 0; j < list_size(player->meta_bodies); j++)
    {
      body_t *meta_body = (body_t *)list_get(player->meta_bodies, j);
      scene_add_body(state->scene_game, meta_body);
      if (j == 0)
      {
        *player->ph_applied_force_magnitude = DRAG_CONST * vec_norm(body_get_velocity(player_get_head(player)));
        create_applied_force(state->scene_game, player->ph_applied_force_magnitude, player_get_head(player));
        create_drag(state->scene_game, DRAG_CONST, player_get_head(player));
      }
      else
      {
        create_drag(state->scene_game, DRAG_CONST, list_get(player->meta_bodies, j));
        create_spring(state->scene_game, SPRING_CONST, meta_body, list_get(player->meta_bodies, j - 1));
      }
    }
  }

  // adds collisions between players:
  for (size_t i = 0; i < GAME_NUM_PLAYERS; i++)
  {
    for (size_t j = 0; j < GAME_NUM_PLAYERS; j++)
    {
      if (i != j)
      {
        body_t *player1_head = player_get_head(list_get(state->players, i));
        player_t *player2 = (player_t *)list_get(state->players, j);

        for (size_t k = 1; k < list_size(player2->meta_bodies); k++)
        {
          body_t *player2_metabody = list_get(player2->meta_bodies, k);
          create_collision(state->scene_game, player1_head, player2_metabody, player_collision_handler, state, NULL);
        }
      }
    }
  }

  // "respawn" (aka init) players
  for (size_t i = 0; i < GAME_NUM_PLAYERS; i++)
  {
    player_t *player = list_get(state->players, i);
    player_respawn(player, state->scene_game);
  }

  // spawn random food
  list_t *pu_types = get_pu_types();
  list_t *pu_colors = get_pu_colors();

  for (size_t i = 0; i < FOOD_COUNT_INITIAL; i++)
  {
    spawn_pellet(state, pu_types, pu_colors);
  }

  // initialize walls
  list_t *wall_left_pts = make_left_wall();
  list_t *wall_left_info = list_init(1, free);
  char *wall_left_name = malloc(sizeof(char) * INFO_MAX_LEN);
  wall_left_name = "wall_left";
  list_add(wall_left_info, wall_left_name);

  list_t *wall_top_pts = make_top_wall();
  list_t *wall_top_info = list_init(1, free);
  char *wall_top_name = malloc(sizeof(char) * INFO_MAX_LEN);
  wall_top_name = "wall_top";
  list_add(wall_top_info, wall_top_name);

  list_t *wall_right_pts = make_right_wall();
  list_t *wall_right_info = list_init(1, free);
  char *wall_right_name = malloc(sizeof(char) * INFO_MAX_LEN);
  wall_right_name = "wall_right";
  list_add(wall_right_info, wall_right_name);

  list_t *wall_bottom_pts = make_bottom_wall();
  list_t *wall_bottom_info = list_init(1, free);
  char *wall_bottom_name = malloc(sizeof(char) * INFO_MAX_LEN);
  wall_bottom_name = "wall_bottom";
  list_add(wall_bottom_info, wall_bottom_name);

  body_t *wall_left = body_init_with_info(wall_left_pts, WALL_MASS, WALL_COLOR, wall_left_info, NULL);
  body_t *wall_top = body_init_with_info(wall_top_pts, WALL_MASS, WALL_COLOR, wall_top_info, NULL);
  body_t *wall_right = body_init_with_info(wall_right_pts, WALL_MASS, WALL_COLOR, wall_right_info, NULL);
  body_t *wall_bottom = body_init_with_info(wall_bottom_pts, WALL_MASS, WALL_COLOR, wall_bottom_info, NULL);

  scene_add_body(state->scene_game, wall_left);
  scene_add_body(state->scene_game, wall_top);
  scene_add_body(state->scene_game, wall_right);
  scene_add_body(state->scene_game, wall_bottom);

  for (size_t k = 0; k < GAME_NUM_PLAYERS; k++)
  {
    create_collision(state->scene_game, player_get_head(list_get(state->players, k)), wall_top, wall_collision_handler, NULL, NULL);
    create_collision(state->scene_game, player_get_head(list_get(state->players, k)), wall_bottom, wall_collision_handler, NULL, NULL);
    create_collision(state->scene_game, player_get_head(list_get(state->players, k)), wall_left, wall_collision_handler, NULL, NULL);
    create_collision(state->scene_game, player_get_head(list_get(state->players, k)), wall_right, wall_collision_handler, NULL, NULL);
  }

  // show player tags
  for (size_t player_id = 0; player_id < list_size(state->players); player_id++)
  {
    player_t *p = (player_t *)list_get(state->players, player_id);
    scene_add_text(state->scene_game, p->score_tag);
  }

  // summon player keybind titles
  char player1_help_text[INFO_MAX_LEN] = "Q W E R";
  char player2_help_text[INFO_MAX_LEN] = "U I O P";
  char player3_help_text[INFO_MAX_LEN] = "Z X C V";
  char player4_help_text[INFO_MAX_LEN] = "B N M <";

  color_t player1_color = ((player_t *) list_get(state->players, 0))->st_color;
  color_t player2_color = ((player_t *) list_get(state->players, 1))->st_color;
  color_t player3_color = ((player_t *) list_get(state->players, 2))->st_color;
  color_t player4_color = ((player_t *) list_get(state->players, 3))->st_color;

  player1_color.r = HELP_OPACITY;
  player2_color.g = HELP_OPACITY;
  player3_color.b = HELP_OPACITY;
  player4_color.a = HELP_OPACITY;

  scene_add_text(state->scene_game, text_init(player1_help_text, PLAYER_1_SETTINGS_CENTER, HELP_HEIGHT, HELP_WIDTH, player1_color, INFINITY));
  scene_add_text(state->scene_game, text_init(player2_help_text, PLAYER_2_SETTINGS_CENTER, HELP_HEIGHT, HELP_WIDTH, player2_color, INFINITY));
  scene_add_text(state->scene_game, text_init(player3_help_text, PLAYER_3_SETTINGS_CENTER, HELP_HEIGHT, HELP_WIDTH, player3_color, INFINITY));
  scene_add_text(state->scene_game, text_init(player4_help_text, PLAYER_4_SETTINGS_CENTER, HELP_HEIGHT, HELP_WIDTH, player4_color, INFINITY));
}

void menu_init(state_t *state)
{
  // init menu
  state->scene_menu = scene_init();
  state->game_started = false;
  state->sound_playing = false;
  state->game_time = 0;

  // spawn players
  state->players = list_init(GAME_NUM_PLAYERS, player_free);
  list_add(state->players, player_init(0, COLOR_BLACK, CENTER, 'q', 'e', 'w', 'r'));
  list_add(state->players, player_init(1, COLOR_BLACK, CENTER, 'u', 'o', 'i', 'p'));
  list_add(state->players, player_init(2, COLOR_BLACK, CENTER, 'z', 'c', 'x', 'v'));
  list_add(state->players, player_init(3, COLOR_BLACK, CENTER, 'b', 'm', 'n', ','));

  // title
  char *title_text = malloc(sizeof(char) * INFO_MAX_LEN);
  vector_t title_pos = vec_add(CENTER, (vector_t){0, 100});
  color_t title_color = SLYCE_COLOR;
  strcpy(title_text, "SLYCE");
  text_t *title = text_init(title_text, title_pos, TITLE_HEIGHT, TITLE_WIDTH, title_color, INFINITY);

  // start button
  char *start_button_text = malloc(sizeof(char) * INFO_MAX_LEN);
  vector_t start_button_pos = vec_add(CENTER, (vector_t){0, -100});
  strcpy(start_button_text, "Type to Start");
  color_t start_button_color = COLOR_WHITE;
  text_t *start_button = text_init(start_button_text, start_button_pos, START_HEIGHT, START_WIDTH, start_button_color, INFINITY);

  // help info
  char *help_text = malloc(sizeof(char) * INFO_MAX_LEN);
  vector_t help_pos = vec_add(start_button_pos, (vector_t){0, -100});
  strcpy(help_text, "Hover to Select Color");
  text_t *help = text_init(help_text, help_pos, HELP_HEIGHT, HELP_WIDTH, HELP_COLOR, INFINITY);

  // player titles
  char *player1_title_text = malloc(sizeof(char) * INFO_MAX_LEN);
  char *player2_title_text = malloc(sizeof(char) * INFO_MAX_LEN);
  char *player3_title_text = malloc(sizeof(char) * INFO_MAX_LEN);
  char *player4_title_text = malloc(sizeof(char) * INFO_MAX_LEN);
  strcpy(player1_title_text, "Player 1");
  strcpy(player2_title_text, "Player 2");
  strcpy(player3_title_text, "Player 3");
  strcpy(player4_title_text, "Player 4");

  vector_t player1_title_pos = vec_add(PLAYER_1_SETTINGS_CENTER, (vector_t){0, 100});
  vector_t player2_title_pos = vec_add(PLAYER_2_SETTINGS_CENTER, (vector_t){0, 100});
  vector_t player3_title_pos = vec_add(PLAYER_3_SETTINGS_CENTER, (vector_t){0, 100});
  vector_t player4_title_pos = vec_add(PLAYER_4_SETTINGS_CENTER, (vector_t){0, 100});

  color_t player_title_color = (color_t){.r = 1, .g = 1, .b = 1, .a = 0.8};

  text_t *player1_title = text_init(player1_title_text, player1_title_pos, PLAYER_TEXT_HEIGHT, PLAYER_TEXT_WIDTH, player_title_color, INFINITY);
  text_t *player2_title = text_init(player2_title_text, player2_title_pos, PLAYER_TEXT_HEIGHT, PLAYER_TEXT_WIDTH, player_title_color, INFINITY);
  text_t *player3_title = text_init(player3_title_text, player3_title_pos, PLAYER_TEXT_HEIGHT, PLAYER_TEXT_WIDTH, player_title_color, INFINITY);
  text_t *player4_title = text_init(player4_title_text, player4_title_pos, PLAYER_TEXT_HEIGHT, PLAYER_TEXT_WIDTH, player_title_color, INFINITY);

  // add all texts
  scene_add_text(state->scene_menu, player1_title);
  scene_add_text(state->scene_menu, player2_title);
  scene_add_text(state->scene_menu, player3_title);
  scene_add_text(state->scene_menu, player4_title);
  scene_add_text(state->scene_menu, title);
  scene_add_text(state->scene_menu, start_button);
  scene_add_text(state->scene_menu, help);

  size_t *p1_id = malloc(sizeof(size_t));
  size_t *p2_id = malloc(sizeof(size_t));
  size_t *p3_id = malloc(sizeof(size_t));
  size_t *p4_id = malloc(sizeof(size_t));
  *p1_id = 0;
  *p2_id = 1;
  *p3_id = 2;
  *p4_id = 3;

  spawn_color_choices(state, p1_id, vec_add(player1_title_pos, CHOICE_SPAWN_POSITION), COLOR_PLAYER1_CHOICE1, COLOR_PLAYER1_CHOICE2, COLOR_PLAYER1_CHOICE3, COLOR_PLAYER1_CHOICE4);
  spawn_color_choices(state, p2_id, vec_add(player2_title_pos, CHOICE_SPAWN_POSITION), COLOR_PLAYER2_CHOICE1, COLOR_PLAYER2_CHOICE2, COLOR_PLAYER2_CHOICE3, COLOR_PLAYER2_CHOICE4);
  spawn_color_choices(state, p3_id, vec_add(player3_title_pos, CHOICE_SPAWN_POSITION), COLOR_PLAYER3_CHOICE1, COLOR_PLAYER3_CHOICE2, COLOR_PLAYER3_CHOICE3, COLOR_PLAYER3_CHOICE4);
  spawn_color_choices(state, p4_id, vec_add(player4_title_pos, CHOICE_SPAWN_POSITION), COLOR_PLAYER4_CHOICE1, COLOR_PLAYER4_CHOICE2, COLOR_PLAYER4_CHOICE3, COLOR_PLAYER4_CHOICE4);
}

void keyboard_handler(state_t *state, char key, key_event_type_t type, double held_time)
{
  if (!state->sound_playing)
  {
    sdl_play_sound(SECONDARY_CHANNEL, "assets/soundtrack.wav", INFINITY);
    state->sound_playing = true;
  }

  if (state->game_started)
  {
    player_t *p = NULL;
    for (size_t player_id = 0; player_id < GAME_NUM_PLAYERS; player_id++)
    {
      player_t *curr_p = list_get(state->players, player_id);
      if (player_moves_on_key(curr_p, key))
      {
        p = curr_p;
        break;
      }
    }

    // if player was not found
    if (p == NULL)
      return;

    // if player was found
    if (p->st_left_key == key)
    {
      if (type == 0 && !p->turn_left)
      {
        p->turn_left = true;
      }
      else if (type == 1)
      {
        p->turn_left = false;
      }
    }
    else if (p->st_right_key == key)
    {
      if (type == 0 && !p->turn_right)
      {
        p->turn_right = true;
      }
      else if (type == 1)
      {
        p->turn_right = false;
      }
    }
    else if (type == 0 && p->st_boost_key == key && p->cd_dash == 0)
    {
      player_dash(p);
    }
    else if (type == 0 && p->st_shoot_key == key && p->cd_shoot == 0)
    {
      body_t *bullet = player_shoot(p);
      for (size_t i = 0; i < scene_bodies(state->scene_game); i++) // create bullet collision with walls
      {
        list_t *body_info = (list_t *)body_get_info(scene_get_body(state->scene_game, i));
        char *body_type = list_get(body_info, 0);

        if (strcmp(body_type, "wall_top") == 0 || strcmp(body_type, "wall_bottom") == 0 || strcmp(body_type, "wall_left") == 0 || strcmp(body_type, "wall_right") == 0)
        {
          create_collision(state->scene_game, bullet, scene_get_body(state->scene_game, i), bullet_collision_handler, state, NULL);
        }
      }
      for (size_t i = 0; i < list_size(state->players); i++) // create bullet collisions with player bodies
      {
        player_t *player = list_get(state->players, i);
        if (p != player)
        {
          list_t *mb = player->meta_bodies;
          for (size_t j = 0; j < list_size(mb); j++)
          {
            body_t *b = list_get(mb, j);
            create_collision(state->scene_game, bullet, b, bullet_collision_handler, state, NULL);
          }
        }
      }
      scene_add_body(state->scene_game, bullet);
    }
  }
  else
  {
    for (size_t i = 0; i < list_size(state->players); i++)
    {
      player_t *p = (player_t *)list_get(state->players, i);
      if (color_equals(p->st_color, (color_t){0, 0, 0, 1}))
      {
        char *error_text = malloc(sizeof(char) * INFO_MAX_LEN);
        strcpy(error_text, "All players must select colors!");

        text_t *error = text_init(error_text, vec_add(CENTER, (vector_t){0, -300}), ERROR_TEXT_HEIGHT, ERROR_TEXT_WIDTH, ERROR_COLOR, ERROR_DURATION);
        sdl_play_sound(REPEATED_SOUND_CHANNEL, "assets/menu_error.wav", 2);
        scene_add_text(state->scene_menu, error);
        return;
      }
    }
    game_init(state);
    sdl_play_sound(FREE_CHANNEL, "assets/game_start.wav", 1);
  }
}
// emscripten: init
state_t *emscripten_init()
{
  // init sdl
  sdl_init(MIN_POSITION, WINDOW);

  // init state
  state_t *state = malloc(sizeof(state_t));

  menu_init(state);

  return state;
}

void handle_mouse(state_t *state)
{
  vector_t mouse_pos = get_mouse_pos();
  // body hovering
  if (!state->game_started)
  {
    list_t *bodies = scene_get_bodies(state->scene_menu);
    for (size_t i = 0; i < scene_bodies(state->scene_menu); i++)
    {
      body_t *b = list_get(bodies, i);
      vector_t body_center = body_get_centroid(b);
      double corrected_x = body_center.x;
      double corrected_y = WINDOW.y - body_center.y;
      if (mouse_pos.x > corrected_x - (CHOICE_SIZE / 2) && mouse_pos.x < corrected_x + (CHOICE_SIZE / 2))
      {
        if (mouse_pos.y > corrected_y - (CHOICE_SIZE / 2) && mouse_pos.y < corrected_y + (CHOICE_SIZE / 2))
        {
          size_t p_id = *((size_t *)body_get_info(b));
          player_t *p = ((player_t *)list_get(state->players, p_id));
          text_t *t = list_get(scene_get_texts(state->scene_menu), p_id);
          color_t color = body_get_color(b);
          if (!color_equals(p->st_color, color))
          {
            player_set_color(p, color);
            text_set_color(t, color);
            sdl_play_sound(FREE_CHANNEL, "assets/menu_select.wav", 0);
          }
        }
      }
    }
  }
}

void main_spawn_pellets(state_t *state)
{
  // random powerup spawns
  list_t *pu_types = get_pu_types();
  list_t *pu_colors = get_pu_colors();

  if (state->time_since_pellet_spawn >= FOOD_SPAWN_TIME) // spawn pellets
  {
    state->time_since_pellet_spawn = 0;
    spawn_pellet(state, pu_types, pu_colors);
  }
}

void main_tick_players(state_t *state)
{
  for (size_t i = 0; i < GAME_NUM_PLAYERS; i++)
  {
    player_t *p = list_get(state->players, i);
    if (player_dying(p))
    {
      player_tick_death(p, state->scene_game);
    }
    player_tick(p, dt);
    player_turn(p);
  }
  scene_tick_canon(state->scene_game, dt);
}

void main_render_game(state_t *state)
{
  // shows cosmetics that are below the bodies in the scene
  for (size_t i = 0; i < GAME_NUM_PLAYERS; i++)
  {
    player_t *p = list_get(state->players, i);
    player_render_cosmetics_below(p);
  }

  // draws all the bodies in a scene
  sdl_render_scene(state->scene_game);
  // scene_draw(state->scene_game);

  // shows cosmetics that are above the bodies in the scene
  for (size_t i = 0; i < GAME_NUM_PLAYERS; i++)
  {
    player_t *p = list_get(state->players, i);
    player_render_cosmetics_above(p);
  }
}

void main_render_menu(state_t *state)
{
  // tick bodies
  sdl_render_scene(state->scene_menu);

  // tick text buttons
  for (size_t i = 0; i < list_size(scene_get_texts(state->scene_menu)); i++)
  {
    text_t *t = list_get(scene_get_texts(state->scene_menu), i);
    if (!t->removed)
    {
      text_tick(t, dt);
    }
  }
}

void emscripten_main(state_t *state)
{
  // sdl: clear window
  sdl_clear();

  // handle mouse
  handle_mouse(state);

  // sdl_render_image();

  // handle keypresses
  sdl_on_key(keyboard_handler);

  if (state->game_started)
  {
    state->game_time += dt;
    state->time_since_pellet_spawn += dt;
    main_spawn_pellets(state);
    main_tick_players(state);
    main_render_game(state);
  }
  else
  {
    sdl_render_image();
    main_render_menu(state);
  }

  // sdl: show
  sdl_show();
}

// emscripten: free resources
void emscripten_free(state_t *state)
{
  scene_free(state->scene_game);
  scene_free(state->scene_menu);
  free(state);
}
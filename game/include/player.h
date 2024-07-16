#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "list.h"
#include "vector.h"
#include "body.h"
#include "color.h"
#include "utils.h"
// #include "physics_constants.h"
#include "string.h"
#include "sdl_wrapper.h"
#include "scene.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct player {
  size_t player_id;
  list_t *meta_bodies;
  text_t *score_tag;

  // physics state
  double *ph_applied_force_magnitude;

  // stats
  double stats_alive_time;
  double stats_periodic_tick;
  size_t stats_kills;
  size_t stats_food;

  // settings
  char st_left_key;
  char st_right_key;
  char st_boost_key;
  char st_shoot_key;
  color_t st_color;

  // player specific state
  bool turn_left;
  bool turn_right;

  bool dying;
  
  // cooldowns
  double cd_dash;
  double cd_shoot;
  double cd_collide_player;
  
  // powerups
  size_t pu_base_speed;
  size_t pu_bullet_speed;
  size_t pu_rotate_rate;
  size_t pu_dash_boost;
} player_t;

player_t *player_init(size_t player_id, color_t color, vector_t pos, char left_key, char right_key, char boost_key, char shoot_key);

body_t *player_get_head(player_t *p);

body_t *player_get_tail(player_t *p);

void player_set_velocity(player_t *p, double vel);

void player_turn(player_t *p);

void player_dash(player_t *p);

body_t *player_shoot(player_t *p);

void player_eat(player_t *p, body_t *food, scene_t *scene);

body_t *player_body(player_t *p);

void player_update_stats(player_t *p);

void player_draw_inner_glow(player_t *p);

void player_render_cosmetics_below(player_t *p);

void player_render_cosmetics_above(player_t *p);

void player_hit(player_t *predator, player_t *prey, body_t *body, scene_t *scene);

vector_t player_head_pos(player_t *p);

vector_t player_tail_pos(player_t *p);

char *player_get_score(player_t *p);

void player_update_kills(player_t *p);

void player_update_food(player_t *p);

body_t *player_add_body(player_t *p);

void player_respawn(player_t *p, scene_t *scene);

void player_free(void *p);

bool player_moves_on_key(player_t *p, char key);

void player_tick(player_t *p, double dt);

bool player_dying(player_t *p);

void player_tick_death(player_t *p, scene_t *scene);

void player_update_pu(player_t *p);

void player_set_color(player_t *p, color_t color);

void player_refresh_cd_dash(player_t *p);

void player_refresh_cd_collide_player(player_t *p);

#endif // #ifndef __PLAYER_H__
#include "player.h"

// constants
const double DEFAULT_BASE_SPEED = 100;
const double DEFAULT_BULLET_SPEED = 200;
const double DEFAULT_TURN_RATE = 0.02;
const double DEFAULT_DASH_BOOST = 200;
const double DEFAULT_DASH_CD = 1.24;
const double DEFAULT_PLAYER_COLLIDE_CD = 0.1;
const double DEFAULT_BULLET_CD = 1;
const double BULLET_SPAWN_DISTANCE = 15;
const double BULLET_SIZE = 7;
const double BULLET_RESOLUTION = 10;
const double BULLET_MASS = 1;
const size_t INFO_MAX_LENGTH = 20;
const double BUFF_SIZE_WIDTH = 15;
const double BUFF_SIZE_HEIGHT = 20;
const size_t CRITICAL_BODIES = 5;
const double INTENSITY_REDUCTION = 0.01;

const double SPEED_PU = 30;
const double ROT_PU = 0.008;
const double BULLET_SPEED_PU = 30;
const double DASH_PU = 150;

const double APPLIED_FORCE_SCALE = 1000.0;

const double CD_COLLISION_INITIAL = 3;
const double CD_SHOOT_INITIAL = 3;

const color_t DEFAULT_COLOR = (color_t){.r = 1, .g = 1, .b = 1, .a = 1};

const double SLUG_SEGMENT_SIZE = 8;
const size_t SLUG_INIT_SEGMENTS = 5;
const double SLUG_MASS = 100;
const double SLUG_RESOLUTION = 20;
const double SLUG_GLOW_RESOLUTION = 10;
const double STARTING_SCORE = 0;

const double SCORE_KILL = 100;
const double SCORE_FOOD = 10;

const double INNER_GLOW_SIZE = 0.75;
const double INNER_GLOW_INTENSITY = 0.95;

const double STATS_SIZE_HEIGHT = 40;
const double STATS_SIZE_WIDTH = 20;
const vector_t STATS_TAG_OFFSET = (vector_t){70, 50};

const double TICK_PERIODIC_DT = 1;

const vector_t SPAWNBOX_MIN = (vector_t){200, 200};
const vector_t SPAWNBOX_MAX = (vector_t){1400, 700};

const double INNER_GLOW_ALPHA = 0.3;

// Creates a circle
list_t *make_round_shape(size_t num_points, double radius, vector_t center)
{
  list_t *circle = list_init(num_points, free);
  double x_pos, y_pos;
  double increment_angle = 2 * M_PI / num_points;
  double angle = 0;
  for (size_t i = 0; i < num_points; i++)
  {
    x_pos = cos(angle) * radius + center.x;
    y_pos = sin(angle) * radius + center.y;
    vector_t *point = malloc(sizeof(vector_t));
    point->x = x_pos;
    point->y = y_pos;
    list_add(circle, point);
    angle += increment_angle;
  }
  return circle;
}

player_t *player_init(size_t player_id, color_t color, vector_t pos, char left_key, char right_key, char boost_key, char shoot_key)
{
  // make segments
  list_t *meta_bodies = list_init(SLUG_INIT_SEGMENTS, free);
  vector_t circ_pos = pos;
  for (size_t i = 0; i < SLUG_INIT_SEGMENTS; i++)
  {
    list_t *curr_circle = make_round_shape(SLUG_RESOLUTION, SLUG_SEGMENT_SIZE, circ_pos);
    list_t *info = list_init(2, free);
    char *body_type = malloc(sizeof(char) * INFO_MAX_LENGTH);
    body_type = "player";
    size_t *id = malloc(sizeof(size_t));
    *id = player_id;
    list_add(info, body_type);
    list_add(info, id);
    body_t *curr_body = body_init_with_info(curr_circle, SLUG_MASS, color, info, free);
    double x_init_vel = rand_range(0, DEFAULT_BASE_SPEED);
    double y_init_vel = sqrt(pow(DEFAULT_BASE_SPEED, 2) - (pow(x_init_vel, 2)));
    body_set_velocity(curr_body, (vector_t){.x = x_init_vel, .y = y_init_vel});
    body_set_glow(curr_body, true);
    body_set_glow_radius(curr_body, SLUG_SEGMENT_SIZE);
    list_add(meta_bodies, curr_body);
  }

  // init player
  player_t *player = malloc(sizeof(player_t));

  player->player_id = player_id;
  player->meta_bodies = meta_bodies;
  player->dying = false;

  player->ph_applied_force_magnitude = malloc(sizeof(double));
  *player->ph_applied_force_magnitude = 0;

  player->stats_alive_time = 0;
  player->stats_kills = 0;
  player->stats_food = 0;
  player->stats_periodic_tick = 0;

  player->st_left_key = left_key;
  player->st_right_key = right_key;
  player->st_boost_key = boost_key;
  player->st_shoot_key = shoot_key;
  player->st_color = color;

  player->turn_left = false;
  player->turn_right = false;

  player->cd_dash = 0;
  player->cd_shoot = CD_SHOOT_INITIAL;
  player->cd_collide_player = CD_COLLISION_INITIAL;

  player->pu_base_speed = 0;
  player->pu_rotate_rate = 0;
  player->pu_bullet_speed = 0;
  player->pu_dash_boost = 0;

  vector_t center = body_get_centroid(player_get_head(player));
  char *score_tag_text = player_get_score(player);
  player->score_tag = text_init(score_tag_text, center, STATS_SIZE_HEIGHT, STATS_SIZE_WIDTH, player->st_color, INFINITY);

  return player;
}

double calc_base_speed(player_t *p)
{
  return DEFAULT_BASE_SPEED + SPEED_PU * sqrt(p->pu_base_speed);
}

double calc_rotate_rate(player_t *p)
{
  return DEFAULT_TURN_RATE + ROT_PU * sqrt(p->pu_rotate_rate);
}

double calc_bullet_speed(player_t *p)
{
  return DEFAULT_BULLET_SPEED + BULLET_SPEED_PU * sqrt(p->pu_bullet_speed);
  ;
}

double calc_dash_boost(player_t *p)
{
  return DEFAULT_DASH_BOOST + DASH_PU * sqrt(p->pu_dash_boost);
}

void player_turn(player_t *p)
{
  if (p->turn_left)
  {
    body_t *player_head = player_get_head(p);
    vector_t head_vector = body_get_velocity(player_head);
    vector_t rotated_vector = vec_rotate(head_vector, calc_rotate_rate(p));
    body_set_velocity(player_head, rotated_vector);
  }
  if (p->turn_right)
  {
    body_t *player_head = player_get_head(p);
    vector_t head_vector = body_get_velocity(player_head);
    vector_t rotated_vector = vec_rotate(head_vector, -calc_rotate_rate(p));
    body_set_velocity(player_head, rotated_vector);
  }
}

void player_dash(player_t *p)
{
  sdl_play_sound(-1, "assets/dash.wav", 0);
  list_t *bodies = p->meta_bodies;
  for (size_t i = 0; i < list_size(bodies); i++)
  {
    body_t *curr_body = (body_t *)list_get(bodies, i);
    vector_t vector_dir = vec_normalize(body_get_velocity(curr_body));
    vector_t updated_velocity = vec_multiply(calc_dash_boost(p) * vec_norm(body_get_velocity(player_get_head(p))), vector_dir);
    body_add_impulse(curr_body, updated_velocity);
  }
  player_refresh_cd_dash(p);
}

void player_eat(player_t *p, body_t *food, scene_t *scene)
{
  list_t *info = (list_t *)body_get_info(food);
  char *body_type = (char *)list_get(info, 0);
  if (strcmp(body_type, "food") != 0)
  {
    return;
  }
  char *pu_type = (char *)list_get(info, 1);

  char *effect = malloc(sizeof(char) * INFO_MAX_LENGTH);
  strcpy(effect, "assets/");
  strcat(effect, pu_type);
  strcat(effect, ".wav");
  sdl_play_sound(-1, effect, 0);
  free(effect);

  player_update_food(p);

  if (strcmp(pu_type, "pu_base_speed") == 0)
  {
    char *text = malloc(sizeof(char) * INFO_MAX_LENGTH);
    strcpy(text, "+MS  \0");
    vector_t center = body_get_centroid(player_get_head(p));
    scene_add_text(scene, text_init(text, center, BUFF_SIZE_HEIGHT, BUFF_SIZE_WIDTH, p->st_color, 1));
    p->pu_base_speed++;
    player_update_pu(p);
  }
  else if (strcmp(pu_type, "pu_bullet_speed") == 0)
  {
    char *text = malloc(sizeof(char) * INFO_MAX_LENGTH);
    strcpy(text, "+BMS \0");
    vector_t center = body_get_centroid(player_get_head(p));
    scene_add_text(scene, text_init(text, center, BUFF_SIZE_HEIGHT, BUFF_SIZE_WIDTH, p->st_color, 1));
    p->pu_bullet_speed++;
    player_update_pu(p);
  }
  else if (strcmp(pu_type, "pu_rotate_rate") == 0)
  {
    char *text = malloc(sizeof(char) * INFO_MAX_LENGTH);
    strcpy(text, "+ROT \0");
    vector_t center = body_get_centroid(player_get_head(p));
    scene_add_text(scene, text_init(text, center, BUFF_SIZE_HEIGHT, BUFF_SIZE_WIDTH, p->st_color, 1));
    p->pu_rotate_rate++;
    player_update_pu(p);
  }
  else if (strcmp(pu_type, "pu_dash_boost") == 0)
  {
    char *text = malloc(sizeof(char) * INFO_MAX_LENGTH);
    strcpy(text, "+DASH\0");
    vector_t center = body_get_centroid(player_get_head(p));
    scene_add_text(scene, text_init(text, center, BUFF_SIZE_HEIGHT, BUFF_SIZE_WIDTH, p->st_color, 1));
    p->pu_dash_boost++;
    player_update_pu(p);
  }
}

void player_update_stats(player_t *p)
{
  // update text
  text_move(p->score_tag, vec_add(body_get_centroid(player_get_head(p)), STATS_TAG_OFFSET));
}

void player_draw_inner_glow(player_t *p)
{
  for (size_t i = 0; i < list_size(p->meta_bodies); i++)
  {
    body_t *curr_body = list_get(p->meta_bodies, i);
    list_t *inner_glow_circle = make_round_shape(SLUG_GLOW_RESOLUTION, INNER_GLOW_SIZE * SLUG_SEGMENT_SIZE, body_get_centroid(curr_body));
    color_t inner_glow_color = body_get_color(curr_body);
    if (inner_glow_color.r != 1)
    {
      inner_glow_color.r = inner_glow_color.r + INNER_GLOW_INTENSITY * (1 - inner_glow_color.r);
    }
    if (inner_glow_color.g != 1)
    {
      inner_glow_color.g = inner_glow_color.g + INNER_GLOW_INTENSITY * (1 - inner_glow_color.g);
    }
    if (inner_glow_color.b != 1)
    {
      inner_glow_color.b = inner_glow_color.b + INNER_GLOW_INTENSITY * (1 - inner_glow_color.b);
    }
    sdl_draw_polygon(inner_glow_circle, inner_glow_color);
    list_free(inner_glow_circle);
  }
}

void player_render_cosmetics_below(player_t *p)
{
  player_update_stats(p);
}

void player_render_cosmetics_above(player_t *p)
{
  player_draw_inner_glow(p);
}

void player_hit(player_t *predator, player_t *prey, body_t *body, scene_t *scene)
{
  sdl_play_sound(-1, "assets/death_dmg.wav", 0);
  // get index of the body that was hit in meta_bodies
  double hit_body_idx = 0;
  for (size_t i = 0; i < list_size(prey->meta_bodies); i++)
  {
    if ((body_t *)list_get(prey->meta_bodies, i) == body)
    {
      hit_body_idx = i;
    }
  }

  if (hit_body_idx > CRITICAL_BODIES - 1)
  {
    // if you hit tail
    // remove tails from scene
    sdl_play_sound(-1, "assets/bullet_hit.wav", 0);
    for (size_t i = hit_body_idx; i < list_size(prey->meta_bodies); i++)
    {
      for (size_t j = 0; j < scene_bodies(scene); j++)
      {
        if ((body_t *)list_get(prey->meta_bodies, i) == scene_get_body(scene, j))
        {
          list_remove(prey->meta_bodies, i);
          i--;
          scene_remove_body(scene, j);
        }
      }
    }
  }
  else
  {
    // if critical strike (death)
    prey->dying = true;
    predator->stats_kills++;
    sdl_play_sound(-1, "assets/death_scream.wav", 0);
  }
}

void player_set_velocity(player_t *p, double vel)
{
  vector_t dir = vec_normalize(body_get_velocity(player_get_head(p)));
  body_set_velocity(player_get_head(p), vec_multiply(vel, dir));
  *p->ph_applied_force_magnitude = APPLIED_FORCE_SCALE * vel;
}

body_t *player_get_head(player_t *p)
{
  return (body_t *)list_get(p->meta_bodies, 0);
}

body_t *player_get_tail(player_t *p)
{
  return (body_t *)list_get(p->meta_bodies, list_size(p->meta_bodies) - 1);
}

vector_t player_head_pos(player_t *p)
{
  return body_get_centroid(player_get_head(p));
}

vector_t player_tail_pos(player_t *p)
{
  return body_get_centroid(player_get_tail(p));
}

void to_string(char *c, size_t s)
{
  sprintf(c, "%zu", s);
}

char *player_get_score(player_t *p)
{
  double score_kills = p->stats_kills * SCORE_KILL;
  double score_food = p->stats_food * SCORE_FOOD;

  size_t score = (size_t)score_kills + score_food;
  char *score_str = malloc(sizeof(char) * INFO_MAX_LENGTH);
  to_string(score_str, score);
  return score_str;
}

void player_update_kills(player_t *p)
{
  p->stats_kills++;
}

void player_update_food(player_t *p)
{
  p->stats_food++;
}

body_t *player_add_body(player_t *p)
{
  vector_t player_tail_pos = body_get_centroid(player_get_tail(p));
  list_t *new_tail = make_round_shape(SLUG_RESOLUTION, SLUG_SEGMENT_SIZE, player_tail_pos);
  list_t *info = list_init(2, free);
  char *body_type = malloc(sizeof(char) * INFO_MAX_LENGTH);
  strcpy(body_type, "player\0");
  size_t *player_id = malloc(sizeof(size_t));
  *player_id = p->player_id;
  list_add(info, body_type);
  list_add(info, player_id);
  body_t *curr_body = body_init_with_info(new_tail, SLUG_MASS, p->st_color, info, NULL);
  body_set_glow(curr_body, true);
  body_set_glow_radius(curr_body, SLUG_SEGMENT_SIZE);
  list_add(p->meta_bodies, curr_body);
  return curr_body;
}

void player_free(void *p)
{
  player_t *p_casted = (player_t *)p;
  free(p_casted->ph_applied_force_magnitude);
  free(p_casted->meta_bodies);
  free(p_casted);
}

bool player_moves_on_key(player_t *p, char key)
{
  return key == p->st_left_key ||
         key == p->st_right_key ||
         key == p->st_boost_key ||
         key == p->st_shoot_key;
}

// ticks a player every couple seconds
void player_tick_periodic(player_t *p)
{
  char *updated_score = player_get_score(p);
  text_edit(p->score_tag, updated_score);
}

void player_tick(player_t *p, double dt)
{
  p->stats_periodic_tick += dt;
  if (p->stats_periodic_tick > TICK_PERIODIC_DT)
  {
    p->stats_periodic_tick = 0;
    player_tick_periodic(p);
  }

  // player stats
  p->stats_alive_time += dt;

  // update cooldowns
  p->cd_dash -= dt;
  p->cd_shoot -= dt;
  p->cd_collide_player -= dt;

  if (p->cd_dash < 0)
    p->cd_dash = 0;
  if (p->cd_shoot < 0)
    p->cd_shoot = 0;
  if (p->cd_collide_player < 0)
    p->cd_collide_player = 0;
}

void player_respawn(player_t *p, scene_t *scene)
{
  // reset all things
  p->stats_kills = 0;
  p->stats_food = 0;
  p->cd_dash = 0;
  p->cd_collide_player = CD_COLLISION_INITIAL;

  p->pu_base_speed = 0;
  p->pu_rotate_rate = 0;
  p->pu_bullet_speed = 0;
  p->pu_dash_boost = 0;

  for (size_t i = 0; i < list_size(p->meta_bodies); i++)
  {
    if (i <= CRITICAL_BODIES - 1)
    {
      for (size_t j = 0; j < scene_bodies(scene); j++)
      {
        if ((body_t *)list_get(p->meta_bodies, i) == scene_get_body(scene, j))
        {
          vector_t spawn_point = (vector_t){rand_range(SPAWNBOX_MIN.x, SPAWNBOX_MAX.x), rand_range(SPAWNBOX_MIN.y, SPAWNBOX_MAX.y)};
          body_set_centroid(scene_get_body(scene, j), spawn_point);
        }
      }
    }
    else
    {
      for (size_t j = 0; j < scene_bodies(scene); j++)
      {
        if ((body_t *)list_get(p->meta_bodies, i) == scene_get_body(scene, j))
        {
          list_remove(p->meta_bodies, i);
          i--;
          scene_remove_body(scene, j);
        }
      }
    }
  }
  p->dying = false;
}

bool player_dying(player_t *p)
{
  return p->dying;
}

void player_tick_death(player_t *p, scene_t *scene)
{
  for (size_t i = 0; i < list_size(p->meta_bodies); i++)
  {
    body_t *curr_body = list_get(p->meta_bodies, i);
    color_t curr_body_color = body_get_color(curr_body);
    curr_body_color.a -= INTENSITY_REDUCTION;
    body_set_color(list_get(p->meta_bodies, i), curr_body_color);
  }
  if (body_get_color(player_get_head(p)).a <= 0)
  {
    player_respawn(p, scene);
    sdl_play_sound(-1, "assets/respawn.wav", 0);
    for (size_t i = 0; i < list_size(p->meta_bodies); i++)
    {
      body_t *curr_body = list_get(p->meta_bodies, i);
      color_t curr_body_color = body_get_color(curr_body);
      curr_body_color.a = 1;
      body_set_color(list_get(p->meta_bodies, i), curr_body_color);
    }
  }
}

void player_update_pu(player_t *p)
{
  player_set_velocity(p, calc_base_speed(p));
}

void player_refresh_cd_collide_player(player_t *p)
{
  p->cd_collide_player = DEFAULT_PLAYER_COLLIDE_CD;
}

void player_refresh_cd_dash(player_t *p)
{
  p->cd_dash = DEFAULT_DASH_CD;
}

void player_refresh_cd_bullet(player_t *p)
{
  p->cd_shoot = DEFAULT_BULLET_CD;
}

void player_set_color(player_t *p, color_t color)
{
  p->st_color = color;
  for (size_t i = 0; i < list_size(p->meta_bodies); i++)
  {
    body_t *curr_body = list_get(p->meta_bodies, i);
    body_set_color(curr_body, color);
  }
  text_set_color(p->score_tag, color);
}

body_t *player_shoot(player_t *p)
{
  sdl_play_sound(-1, "assets/shoot.wav", 0);
  body_t *head = player_get_head(p);
  vector_t bullet_direction = vec_normalize(body_get_velocity(head));
  vector_t bullet_spawn_position = vec_add(body_get_centroid(head), vec_multiply(BULLET_SPAWN_DISTANCE, bullet_direction));
  vector_t bullet_velocity = vec_multiply(calc_bullet_speed(p), bullet_direction);
  list_t *new_bullet = make_round_shape(BULLET_RESOLUTION, BULLET_SIZE, bullet_spawn_position);
  char *body_type = malloc(sizeof(char) * INFO_MAX_LENGTH);
  body_type = "bullet";
  size_t *id = malloc(sizeof(size_t));
  *id = p->player_id;
  list_t *info = list_init(2, free);
  list_add(info, body_type);
  list_add(info, id);
  body_t *bullet = body_init_with_info(new_bullet, BULLET_MASS, p->st_color, info, NULL);
  body_set_velocity(bullet, bullet_velocity);
  player_refresh_cd_bullet(p);
  return bullet;
}
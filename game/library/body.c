#include "body.h"

#include "collision.h"
#include "color.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include "utils.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const double DEV_MODE_VECTOR_SCALE = 5;
const double DEV_MODE_VECTOR_THICKNESS = 1;
color_t DEV_MODE_VEL_COLOR = (color_t){.r = 0, .g = 1, .b = 0, .a = 1};
color_t DEV_MODE_ACL_COLOR = (color_t){.r = 1, .g = 0, .b = 0, .a = 1};

const double GLOW_SCALE = 0.1;
const size_t GLOW_FACTOR = 8;
const double GLOW_REDUCTION = 0.8;
const double GLOW_RESOLUTION = 10;
const double GLOW_INCREASE = 3;

typedef struct body {
  color_t color;
  list_t *shape;
  vector_t pos; // position
  vector_t vel; // velocity
  vector_t acl; // acceleration
  double mass;
  vector_t centroid;
  vector_t impulse;
  double angle;
  bool remove;
  bool glowing;
  void *info;
  double glow_radius;
  free_func_t info_freer;
} body_t;

body_t *body_init(list_t *shape, double mass, color_t color) {
  body_t *new_body = malloc(sizeof(body_t));
  new_body->color = color;
  new_body->shape = shape;
  new_body->pos = VEC_ZERO;
  new_body->vel = VEC_ZERO;
  new_body->acl = VEC_ZERO;
  new_body->impulse = VEC_ZERO;
  new_body->mass = mass;
  new_body->centroid = polygon_centroid(shape);
  new_body->angle = 0;
  new_body->remove = false;
  new_body->info = NULL;
  new_body->info_freer = NULL;
  new_body->glowing = false;
  new_body->glow_radius = 0;
  return new_body;
}

body_t *body_init_with_info(list_t *shape, double mass, color_t color,
                            void *info, free_func_t info_freer) {
  body_t *body = body_init(shape, mass, color);
  body->info = info;
  body->info_freer = info_freer;
  return body;
}

void body_free(void *body) {
  body_t *body_casted = (body_t *)body;
  list_free(body_casted->shape);
  if (body_casted->info_freer != NULL) {
    body_casted->info_freer(body_casted->info);
  }
  free(body_casted);
}

void *body_get_info(body_t *body) { return body->info; }

list_t *body_get_shape(body_t *body) {
  list_t *new_body = list_init(list_size(body->shape), free);
  list_t *body_pts = body->shape;
  for (size_t i = 0; i < list_size(body_pts); i++) {
    vector_t *curr_vec = list_get(body_pts, i);
    vector_t *new_vec = malloc(sizeof(vector_t));
    new_vec->x = curr_vec->x;
    new_vec->y = curr_vec->y;
    list_add(new_body, new_vec);
  }
  return new_body;
}

double body_get_mass(body_t *body) { return body->mass; }

vector_t body_get_centroid(body_t *body) { return body->centroid; }

vector_t body_get_position(body_t *body) { return body->pos; }

void body_set_position(body_t *body, vector_t pos) { body->pos = pos; }

vector_t body_get_velocity(body_t *body) { return body->vel; }

void body_set_velocity(body_t *body, vector_t v) { body->vel = v; }

vector_t body_get_acceleration(body_t *body) { return body->acl; }

void body_set_acceleration(body_t *body, vector_t new_acl) {
  body->acl = new_acl;
}

color_t body_get_color(body_t *body) { return body->color; }

void body_set_centroid(body_t *body, vector_t x) {
  vector_t dx = vec_subtract(x, body->centroid);
  polygon_translate(body->shape, dx);
  body->centroid = x;
}

void body_set_color(body_t *body, color_t color) { body->color = color; }

void body_set_rotation(body_t *body, double angle) {
  polygon_rotate(body->shape, angle - body->angle, body->centroid);
  body->angle = angle;
}

void body_add_force(body_t *body, vector_t force) {
  vector_t a = body_get_acceleration(body);
  vector_t da = vec_multiply(1.0 / body_get_mass(body), force);
  body_set_acceleration(body, vec_add(a, da));
}

list_t *vector_pts(vector_t start, vector_t acl) {
  vector_t end = vec_add(start, acl);
  list_t *pts = list_init(4, free);
  vector_t *lower_left = malloc(sizeof(vector_t));
  lower_left->x = start.x - DEV_MODE_VECTOR_THICKNESS;
  lower_left->y = start.y - DEV_MODE_VECTOR_THICKNESS;
  list_add(pts, lower_left);

  vector_t *upper_left = malloc(sizeof(vector_t));
  upper_left->x = start.x - DEV_MODE_VECTOR_THICKNESS;
  upper_left->y = start.y + DEV_MODE_VECTOR_THICKNESS;
  list_add(pts, upper_left);

  vector_t *upper_right = malloc(sizeof(vector_t));
  upper_right->x = end.x + DEV_MODE_VECTOR_THICKNESS;
  upper_right->y = end.y + DEV_MODE_VECTOR_THICKNESS;
  list_add(pts, upper_right);

  vector_t *lower_right = malloc(sizeof(vector_t));
  lower_right->x = end.x + DEV_MODE_VECTOR_THICKNESS;
  lower_right->y = end.y - DEV_MODE_VECTOR_THICKNESS;
  list_add(pts, lower_right);

  return pts;
}

void body_draw_acl(body_t *body) {
  vector_t start = body_get_centroid(body);
  vector_t a = body_get_acceleration(body);
  vector_t a_norm = vec_normalize(a);
  double length = vec_norm(a);
  vector_t a_log = vec_multiply(DEV_MODE_VECTOR_SCALE * log(length), a_norm);
  if (length != 0) {
    sdl_draw_polygon(vector_pts(start, a_log), DEV_MODE_ACL_COLOR);
  }
}

void body_add_elastic_impulse(body_t *body1, body_t *body2, double elasticity) {
  double reduced_mass;
  if (body1->mass == INFINITY) {
    reduced_mass = body2->mass;
  } else if (body2->mass == INFINITY) {
    reduced_mass = body1->mass;
  } else {
    reduced_mass = (body1->mass * body2->mass) / (body1->mass + body2->mass);
  }
  vector_t collision_axis = find_collision(body1->shape, body2->shape).axis;
  vector_t centroid_diff =
      vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
  if (vec_dot(collision_axis, centroid_diff) < 0) {
    collision_axis = vec_negate(collision_axis);
  }
  double u_a = vec_dot(body1->vel, collision_axis);
  double u_b = vec_dot(body2->vel, collision_axis);
  double c_r = elasticity;

  double impulse_scalar = reduced_mass * (1 + c_r) * (u_b - u_a);
  body1->impulse =
      vec_add(body1->impulse, vec_multiply(impulse_scalar, collision_axis));
  body2->impulse =
      vec_add(body2->impulse, vec_multiply(-impulse_scalar, collision_axis));
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->impulse = vec_add(body->impulse, impulse);
}

bool body_get_glow(body_t *body) {
  return body->glowing;
}

void body_set_glow(body_t *body, bool glow) {
  body->glowing = glow;
}

double body_get_glow_radius(body_t *body) {
  return body->glow_radius;
}

void body_set_glow_radius(body_t *body, double glow_radius) {
  body->glow_radius = glow_radius;
}

void body_draw_glow(body_t *body, double radius) {
  color_t glow_color = body_get_color(body);
  glow_color.a = GLOW_SCALE * body_get_color(body).a; 
  for (size_t j = 0; j < GLOW_FACTOR; j++) { 
    glow_color.a = GLOW_REDUCTION * glow_color.a;
    list_t *glow_circle = make_circle(GLOW_RESOLUTION, radius + GLOW_INCREASE*j, body_get_centroid(body)); 
    sdl_draw_polygon(glow_circle, glow_color);
    list_free(glow_circle);
  }
}

void body_tick(body_t *body, double dt) {
  vector_t old_vel = body->vel;
  vector_t new_vel =
      vec_add(old_vel, vec_multiply(dt, body_get_acceleration(body)));
  vector_t impulse_to_add =
      vec_multiply(1.0 / body_get_mass(body), body->impulse);
  new_vel = vec_add(new_vel, impulse_to_add);
  body->vel = new_vel;
  vector_t avg_vel = vec_multiply(0.5, vec_add(old_vel, new_vel));
  vector_t pos_change = vec_multiply(dt, avg_vel);
  body->pos = vec_add(body->pos, pos_change);
  body->centroid = vec_add(body->centroid, pos_change);
  polygon_translate(body->shape, pos_change);
  body_set_acceleration(body, VEC_ZERO);
  body->impulse = VEC_ZERO;
}

void body_tick_canon(body_t *body, double dt) {
  vector_t old_vel = body->vel;
  vector_t new_vel =
      vec_add(old_vel, vec_multiply(dt, body_get_acceleration(body)));
  vector_t impulse_to_add =
      vec_multiply(1.0 / body_get_mass(body), body->impulse);
  new_vel = vec_add(new_vel, impulse_to_add);
  body->vel = new_vel;
  vector_t pos_change = vec_multiply(dt, new_vel);
  body->pos = vec_add(body->pos, pos_change);
  body->centroid = vec_add(body->centroid, pos_change);
  polygon_translate(body->shape, pos_change);
  body_set_acceleration(body, VEC_ZERO);
  body->impulse = VEC_ZERO;
}

void body_tick_canon_no_reset(body_t *body, double dt) {
  vector_t old_vel = body->vel;
  vector_t new_vel =
      vec_add(old_vel, vec_multiply(dt, body_get_acceleration(body)));
  vector_t impulse_to_add =
      vec_multiply(1.0 / body_get_mass(body), body->impulse);
  new_vel = vec_add(new_vel, impulse_to_add);
  body->vel = new_vel;
  vector_t pos_change = vec_multiply(dt, new_vel);
  body->pos = vec_add(body->pos, pos_change);
  body->centroid = vec_add(body->centroid, pos_change);
  polygon_translate(body->shape, pos_change);
  body->impulse = VEC_ZERO;
}

void body_remove(body_t *body) { body->remove = true; }

bool body_is_removed(body_t *body) { return body->remove; }
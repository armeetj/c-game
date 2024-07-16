#include "body.h"
#include "polygon.h"
#include "vector.h"
#include "sdl_wrapper.h"
#include "color.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

const double DEV_MODE_VECTOR_SCALE = 5;
const double DEV_MODE_VECTOR_THICKNESS = 1;
rgb_color_t DEV_MODE_VEL_COLOR = (rgb_color_t){.r = 0, .g = 1, .b = 0};
rgb_color_t DEV_MODE_ACL_COLOR = (rgb_color_t) {.r = 1, .g = 0, .b = 0};

typedef struct body {
  rgb_color_t color;
  list_t *shape;
  vector_t pos; // position
  vector_t vel; // velocity
  vector_t acl; // acceleration
  double mass;
  vector_t centroid;
  double angle;
  list_t *properties;
} body_t;

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  body_t *new_body = malloc(sizeof(body_t));
  new_body->color = color;
  new_body->shape = shape;
  new_body->pos = VEC_ZERO;
  new_body->vel = VEC_ZERO;
  new_body->acl = VEC_ZERO;
  new_body->mass = mass;
  new_body->centroid = polygon_centroid(shape);
  new_body->angle = 0;
  new_body->properties = list_init(0, free);
  return new_body;
}

void body_free(void *body) {
  body_t *body_casted = (body_t *) body;
  list_free(body_casted->shape);
  list_free(body_casted->properties);
  free(body);
}

list_t *body_get_properties(body_t *body) {
  return body->properties;
}

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

void body_set_position(body_t *body, vector_t pos) { 
  body->pos = pos; 
  body->centroid = pos;
}

vector_t body_get_velocity(body_t *body) { return body->vel; }

void body_set_velocity(body_t *body, vector_t v) { body->vel = v; }

vector_t body_get_acceleration(body_t *body) { return body->acl; }

void body_set_acceleration(body_t *body, vector_t new_acl) {
  body->acl = new_acl;
}

rgb_color_t body_get_color(body_t *body) { return body->color; }

void body_set_centroid(body_t *body, vector_t x) { body->centroid = x; }

void body_set_color(body_t *body, rgb_color_t color) {
  body->color = color;
}

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
  lower_left->x = start.x -DEV_MODE_VECTOR_THICKNESS;
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
  vector_t start = body_get_position(body);
  vector_t a = body_get_acceleration(body);
  vector_t a_norm = vec_normalize(a);
  double length = vec_norm(a);
  vector_t a_log = vec_multiply(DEV_MODE_VECTOR_SCALE * log(length), a_norm);
  sdl_draw_polygon(vector_pts(start, a), DEV_MODE_ACL_COLOR);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  vector_t velocity_body_curr = body_get_velocity(body);
  vector_t impulse_to_add = vec_multiply(1.0 / body_get_mass(body), impulse);
  body_set_velocity(body, vec_add(velocity_body_curr, impulse_to_add));
}

void body_tick(body_t *body, double dt) {
  body->vel = vec_add(body->vel, vec_multiply(dt, body_get_acceleration(body)));
  vector_t delta_x = vec_multiply(dt, body->vel);
  body->pos = vec_add(body->pos, delta_x);
  body->centroid = vec_add(body->centroid, delta_x);
  polygon_translate(body->shape, delta_x);
}

#include "body.h"
#include "polygon.h"
#include "vector.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct body {
  rgb_color_t color;
  list_t *shape;
  vector_t pos; // position
  vector_t vel; // velocity
  vector_t acl; // acceleration
  double mass;
  vector_t centroid;
  double angle;
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
  return new_body;
}

void body_free(body_t *body) {
  free(body->shape);
  free(body);
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

vector_t body_get_centroid(body_t *body) { return body->centroid; }

vector_t body_get_position(body_t *body) { return body->pos; }

void body_set_position(body_t *body, vector_t pos) { body->pos = pos; }

vector_t body_get_velocity(body_t *body) { return body->vel; }

void body_set_velocity(body_t *body, vector_t v) { body->vel = v; }

vector_t body_get_acceleration(body_t *body) { return body->acl; }

void body_set_acceleration(body_t *body, vector_t new_acl) {
  body->acl = new_acl;
}

rgb_color_t body_get_color(body_t *body) { return body->color; }

void body_set_centroid(body_t *body, vector_t x) { body->centroid = x; }

void body_set_rotation(body_t *body, double angle) {
  polygon_rotate(body->shape, angle - body->angle, body->centroid);
  body->angle = angle;
}

void body_tick(body_t *body, double dt) {
  vector_t vel_term = vec_multiply(dt, body_get_velocity(body));
  vector_t acl_term =
      vec_multiply(0.5, vec_multiply(pow(dt, 2), body_get_acceleration(body)));
  vector_t delta_x = vec_add(vel_term, acl_term);
  body->pos = vec_add(body->pos, delta_x);
  body->vel = vec_add(body->vel, vec_multiply(dt, body_get_acceleration(body)));
  polygon_translate(body->shape, delta_x);
  body_set_centroid(body, vec_add(body->centroid, delta_x));
}

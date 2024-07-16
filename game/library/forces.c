#include "forces.h"
#include "aux.h"
#include "body.h"
#include "collision.h"
#include "collision_package.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const double MIN_DIST = 30;

void gravity_force_creator(void *aux) {
  aux_t *aux_casted = (aux_t *)aux;
  double G = *((double *)list_get(aux_get_constants(aux_casted), 0));
  body_t *body1 = (body_t *)list_get(aux_get_bodies(aux_casted), 0);
  body_t *body2 = (body_t *)list_get(aux_get_bodies(aux_casted), 1);
  vector_t v1 = body_get_centroid(body1);
  vector_t v2 = body_get_centroid(body2);
  double dist = vec_dist(v2, v1);
  if (dist >= MIN_DIST) {
    vector_t r = vec_subtract(v2, v1);
    vector_t r_hat = vec_multiply(1.0 / vec_norm(r), r);
    double numerator = G * body_get_mass(body1) * body_get_mass(body2);
    double denominator = vec_norm(r) * vec_norm(r);
    double g_scal = numerator / denominator;
    vector_t g_vec = vec_multiply(g_scal, r_hat);
    body_add_force(body1, g_vec);
    body_add_force(body2, vec_negate(g_vec));
  }
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  force_creator_t g_creator = gravity_force_creator;

  list_t *constants = list_init(1, free);
  double *G_address = malloc(sizeof(double));
  *G_address = G;
  list_add(constants, G_address);

  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);

  aux_t *g_aux = aux_init(constants, bodies);
  scene_add_bodies_force_creator(scene, g_creator, g_aux, bodies, aux_free);
  // scene_add_force_creator(scene, g_creator, g_aux, aux_free);
}

void spring_force_creator(void *aux) {
  aux_t *aux_casted = (aux_t *)aux;

  double k = *((double *)list_get(aux_get_constants(aux_casted), 0));
  body_t *body1 = (body_t *)list_get(aux_get_bodies(aux_casted), 0);
  body_t *body2 = (body_t *)list_get(aux_get_bodies(aux_casted), 1);

  vector_t distance =
      vec_subtract(body_get_centroid(body1), body_get_centroid(body2));
  vector_t s_vec = vec_multiply(-k, distance);
  body_add_force(body1, s_vec);
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  force_creator_t s_creator = spring_force_creator;

  list_t *constants = list_init(1, free);
  double *k_address = malloc(sizeof(double));
  *k_address = k;
  list_add(constants, k_address);

  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);

  aux_t *s_aux = aux_init(constants, bodies);
  scene_add_bodies_force_creator(scene, s_creator, s_aux, bodies, aux_free);
}

void drag_force_creator(void *aux) {
  aux_t *aux_casted = (aux_t *)aux;
  double gamma = *((double *)list_get(aux_get_constants(aux_casted), 0));
  body_t *body = (body_t *)list_get(aux_get_bodies(aux_casted), 0);

  vector_t vel = body_get_velocity(body);
  vector_t d_vec = vec_multiply(-gamma, vel);
  body_add_force(body, d_vec);
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
  force_creator_t d_creator = drag_force_creator;

  list_t *constants = list_init(1, free);
  double *gamma_address = malloc(sizeof(double));
  *gamma_address = gamma;
  list_add(constants, gamma_address);

  list_t *bodies = list_init(1, NULL);
  list_add(bodies, body);

  aux_t *d_aux = aux_init(constants, bodies);
  scene_add_bodies_force_creator(scene, d_creator, d_aux, bodies, aux_free);
}

void applied_force_creator(void *aux) {
  aux_t *aux_casted = (aux_t *)aux;
  double magnitude = *((double *)list_get(aux_get_constants(aux_casted), 0));
  body_t *body = (body_t *)list_get(aux_get_bodies(aux_casted), 0);

  vector_t force = vec_multiply(magnitude, vec_normalize(body_get_velocity(body)));
  body_add_force(body, force);
}

void create_applied_force(scene_t *scene, double *magnitude, body_t *body) {
  force_creator_t a_creator = applied_force_creator;

  list_t *constants = list_init(1, free);
  list_add(constants, magnitude);

  list_t *bodies = list_init(1, NULL);
  list_add(bodies, body);

  aux_t *a_aux = aux_init(constants, bodies);
  scene_add_bodies_force_creator(scene, a_creator, a_aux, bodies, aux_free);
}

void destructive_collision_creator(void *aux) {
  aux_t *aux_casted = (aux_t *)aux;
  body_t *body1 = (body_t *)list_get(aux_get_bodies(aux_casted), 0);
  body_t *body2 = (body_t *)list_get(aux_get_bodies(aux_casted), 1);
  list_t *shape1 = body_get_shape(body1);
  list_t *shape2 = body_get_shape(body2);
  if (find_collision(shape1, shape2).collided) {
    body_remove(body1);
    body_remove(body2);
  }
  list_free(shape1);
  list_free(shape2);
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  force_creator_t creator = destructive_collision_creator;

  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);

  aux_t *aux = aux_init(NULL, bodies);
  scene_add_bodies_force_creator(scene, creator, aux, bodies, aux_free);
}

void general_collision_handler(void *pkg) {
  collision_package_t *pkg_casted = (collision_package_t *)pkg;
  collision_package_handle(pkg_casted);
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  collision_package_t *pkg =
      collision_package_init(body1, body2, handler, aux, freer);
  force_creator_t collision_handler = general_collision_handler;

  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);

  scene_add_bodies_force_creator(scene, collision_handler, pkg, bodies,
                                 collision_package_free);
}

void normal_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                              void *aux) {
  list_t *info = (list_t *)aux;
  bool *impulsed_last_tick = list_get(info, 0);
  double *elasticity = list_get(info, 1);

  list_t *shape1 = body_get_shape(body1);
  list_t *shape2 = body_get_shape(body2);

  if (find_collision(shape1, shape2).collided && !(*impulsed_last_tick)) {
    *impulsed_last_tick = true;
    body_add_elastic_impulse(body1, body2, *elasticity);
  } else {
    *impulsed_last_tick = false;
  }
  list_free(shape1);
  list_free(shape2);
}

void create_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2) {

  force_creator_t handler = general_collision_handler;
  list_t *info = list_init(2, NULL);

  bool *impulsed_last_tick = malloc(sizeof(bool));
  *impulsed_last_tick = false;
  list_add(info, impulsed_last_tick);

  double *e = malloc(sizeof(double));
  *e = elasticity;
  list_add(info, e);

  collision_package_t *pkg = collision_package_init(
      body1, body2, normal_collision_handler, info, NULL); // CHECK FREER

  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);

  scene_add_bodies_force_creator(scene, handler, pkg, bodies,
                                 collision_package_free);
}
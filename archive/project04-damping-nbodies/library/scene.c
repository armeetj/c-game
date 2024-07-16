#include "scene.h"
#include "body.h"
#include "force_wrapper.h"
#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

const size_t DEFAULT_NUM_BODIES = 50;
const size_t DEFAULT_NUM_FORCES = 20;

typedef struct scene {
  list_t *bodies;
  list_t *forces;
  double time_s;
  bool dev_mode;
} scene_t;

scene_t *scene_init(void) {
  scene_t *s = malloc(sizeof(scene_t));
  s->bodies = list_init(DEFAULT_NUM_BODIES, body_free);
  s->forces = list_init(DEFAULT_NUM_FORCES, force_free);
  s->time_s = 0;
  s->dev_mode = false;
  return s;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->forces);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->bodies); }

body_t *scene_get_body(scene_t *scene, size_t index) {
  return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index) {
  list_remove(scene->bodies, index);
}

bool scene_get_dev_mode(scene_t *scene) {
  return scene->dev_mode;
}

void scene_set_dev_mode(scene_t *scene, bool dev_mode) {
  scene->dev_mode = dev_mode;
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux, free_func_t freer) {
  force_wrapper_t *force = force_init(forcer, aux, freer);
  list_add(scene->forces, force);
}

void scene_draw(scene_t *scene) {
  for(size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *body = scene_get_body(scene, i);
    sdl_draw_polygon(body_get_shape(body), body_get_color(body));
    if (scene->dev_mode) {
      body_draw_acl(body);
    }
  }
}

void scene_tick(scene_t *scene, double dt) {
  scene->time_s += dt;
  for (size_t j = 0; j < list_size(scene->forces); j++) {
    force_create(list_get(scene->forces, j));
  }
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *curr_body = scene_get_body(scene, i);
    body_tick(curr_body, dt);
  }
}

void scene_accel_reset(scene_t *scene) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *curr_body = scene_get_body(scene, i);
    body_set_acceleration(curr_body, (vector_t) {.x = 0, .y = 0});
  }
}
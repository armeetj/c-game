#include "scene.h"
#include "body.h"
#include "state.h"
#include <stdio.h>
#include <stdlib.h>

const size_t DEFAULT_NUM_BODIES = 10;

typedef struct scene {
  list_t *bodies;
  double time_s;
} scene_t;

scene_t *scene_init(void) {
  scene_t *s = malloc(sizeof(scene_t));
  s->bodies = list_init(DEFAULT_NUM_BODIES, free);
  s->time_s = 0;
  return s;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
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

void scene_tick(scene_t *scene, double dt) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_tick(scene_get_body(scene, i), dt);
    scene->time_s += dt;
  }
}

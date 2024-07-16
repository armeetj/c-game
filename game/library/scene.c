#include "scene.h"
#include "aux.h"
#include "body.h"
#include "force_wrapper.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const size_t DEFAULT_NUM_BODIES = 50;
const size_t DEFAULT_NUM_TEXTS = 10;
const size_t DEFAULT_NUM_FORCES = 20;

typedef struct scene {
  list_t *bodies;
  list_t *texts;
  list_t *forces;
  double time_s;
  bool dev_mode;
} scene_t;

scene_t *scene_init(void) {
  scene_t *s = malloc(sizeof(scene_t));
  s->bodies = list_init(DEFAULT_NUM_BODIES, body_free);
  s->texts = list_init(DEFAULT_NUM_TEXTS, text_free);
  s->forces = list_init(DEFAULT_NUM_FORCES, force_free);
  s->time_s = 0;
  s->dev_mode = false;
  return s;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->forces);
  list_free(scene->texts);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->bodies); }

list_t *scene_get_bodies(scene_t *scene) { return scene->bodies; }

list_t *scene_get_texts(scene_t *scene) {
  return scene->texts;
}

body_t *scene_get_body(scene_t *scene, size_t index) {
  return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
}

void scene_add_text(scene_t *scene, text_t *text) {
  list_add(scene->texts, text);
}

void scene_remove_body(scene_t *scene, size_t index) {
  body_remove(list_get(scene->bodies, index));
}

bool scene_get_dev_mode(scene_t *scene) { return scene->dev_mode; }

void scene_set_dev_mode(scene_t *scene, bool dev_mode) {
  scene->dev_mode = dev_mode;
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  force_wrapper_t *force = force_init(forcer, aux, freer);
  list_add(scene->forces, force);
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer) {
  force_wrapper_t *force = force_init_with_bodies(forcer, aux, freer, bodies);
  list_add(scene->forces, force);
}

void scene_remove_forces_from_body(scene_t *scene, body_t *body) {
  for (size_t i = 0; i < list_size(scene->forces); i++) {
    force_wrapper_t *force = list_get(scene->forces, i);
    list_t *bodies = force_get_bodies(force);
    for (size_t j = 0; j < list_size(bodies); j++) {
      if (list_get(bodies, j) == body) {
        force_remove(list_get(scene->forces, i));
        break;
      }
    }
  }
}

void scene_draw(scene_t *scene) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
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
    if (body_is_removed(curr_body)) {
      scene_remove_forces_from_body(scene, curr_body);
      body_t *removed_body = list_remove(scene->bodies, i);
      body_free(removed_body);
      i--;
    } else {
      body_tick(curr_body, dt);
    }
  }
  for (size_t i = 0; i < list_size(scene->forces); i++) {
    force_wrapper_t *curr_force = list_get(scene->forces, i);
    if (force_is_removed(curr_force)) {
      force_wrapper_t *removed_force = list_remove(scene->forces, i);
      force_free(removed_force);
      i--;
    }
  }
}

void scene_tick_canon(scene_t *scene, double dt) {
  scene->time_s += dt;
  // forces tick
  for (size_t j = 0; j < list_size(scene->forces); j++) {
    force_create(list_get(scene->forces, j));
  }
  // body tick
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *curr_body = scene_get_body(scene, i);
    if (body_is_removed(curr_body)) {
      scene_remove_forces_from_body(scene, curr_body);
      body_t *removed_body = list_remove(scene->bodies, i);
      body_free(removed_body);
      i--;
    } else {
      body_tick_canon(curr_body, dt);
      if (body_get_glow(curr_body)) body_draw_glow(curr_body, body_get_glow_radius(curr_body));
    }
  }
  for (size_t i = 0; i < list_size(scene->forces); i++) {
    force_wrapper_t *curr_force = list_get(scene->forces, i);
    if (force_is_removed(curr_force)) {
      force_wrapper_t *removed_force = list_remove(scene->forces, i);
      force_free(removed_force);
      i--;
    }
  }

  // texts tick
  for (size_t i = 0; i < list_size(scene->texts); i++) {
    text_t *t = list_get(scene->texts, i);
    if(!t->removed) {
      text_tick(t, dt);
    }
  }
}

void scene_tick_canon_no_reset(scene_t *scene, double dt) {
  scene->time_s += dt;
  for (size_t j = 0; j < list_size(scene->forces); j++) {
    force_create(list_get(scene->forces, j));
  }
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *curr_body = scene_get_body(scene, i);
    if (body_is_removed(curr_body)) {
      scene_remove_forces_from_body(scene, curr_body);
      body_t *removed_body = list_remove(scene->bodies, i);
      body_free(removed_body);
      i--;
      body_tick_canon_no_reset(curr_body, dt);
    }
  }
    for (size_t i = 0; i < list_size(scene->forces); i++) {
    force_wrapper_t *curr_force = list_get(scene->forces, i);
    if (force_is_removed(curr_force)) {
      force_wrapper_t *removed_force = list_remove(scene->forces, i);
      force_free(removed_force);
      i--;
    }
  }
}

void scene_accel_reset(scene_t *scene) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *curr_body = scene_get_body(scene, i);
    body_set_acceleration(curr_body, (vector_t){.x = 0, .y = 0});
  }
}
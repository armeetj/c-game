#include "force_wrapper.h"

#include "aux.h"
#include "list.h"
#include <stdbool.h>
#include <stdlib.h>

const size_t NUM_BODIES = 20;

typedef struct force_wrapper {
  force_creator_t force_creator;
  void *aux;
  bool remove;
  free_func_t freer;
  list_t *bodies;
} force_wrapper_t;

force_wrapper_t *force_init(force_creator_t force_creator, void *aux,
                            free_func_t freer) {
  force_wrapper_t *force = malloc(sizeof(force_wrapper_t));
  force->force_creator = force_creator;
  force->aux = aux;
  force->remove = false;
  force->freer = freer;
  force->bodies = NULL;
  return force;
}

force_wrapper_t *force_init_with_bodies(force_creator_t force_creator,
                                        void *aux, free_func_t freer,
                                        list_t *bodies) {
  force_wrapper_t *force = force_init(force_creator, aux, freer);
  force->bodies = bodies;
  return force;
}

list_t *force_get_bodies(force_wrapper_t *f) { return f->bodies; }

void force_create(force_wrapper_t *f) { f->force_creator(f->aux); }

void *force_get_aux(force_wrapper_t *force) { return force->aux; }

void force_set_aux(force_wrapper_t *force, void *aux) { force->aux = aux; }

void force_remove(force_wrapper_t *force) { force->remove = true; }

bool force_is_removed(force_wrapper_t *force) { return force->remove; }

void force_free(void *force) {
  force_wrapper_t *casted_force = (force_wrapper_t *)force;
  if (casted_force->freer != NULL) {
    casted_force->freer(casted_force->aux);
  } else if (casted_force->bodies != NULL) {
    list_free(casted_force->bodies);
  }
  free(casted_force);
}
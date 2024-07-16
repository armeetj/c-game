#include "force_wrapper.h"
#include "aux.h"
#include <stdlib.h>

typedef struct force_wrapper {
    force_creator_t force_creator;
    void *aux;
    free_func_t freer;
} force_wrapper_t;

force_wrapper_t *force_init(force_creator_t force_creator, void *aux, free_func_t freer) {
    force_wrapper_t *force = malloc(sizeof(force_wrapper_t));
    force->force_creator = force_creator;
    force->aux = aux;
    force->freer = freer;
    return force;
}

void force_create(force_wrapper_t *f) {
    f->force_creator(f->aux);
}

void *force_get_aux(force_wrapper_t *force) {
    return force->aux;
}

void force_set_aux(force_wrapper_t *force, void *aux) {
    force->aux = aux;
}

void force_free(void *aux) {
    force_wrapper_t *force = (force_wrapper_t *) aux;
    aux_free(force->aux);
    free(force);
}
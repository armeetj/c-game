#include "aux.h"
#include "list.h"

typedef struct aux {
  list_t *constants;
  list_t *bodies;
} aux_t;

aux_t *aux_init(list_t *constants, list_t *bodies) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->constants = constants;
  aux->bodies = bodies;
  return aux;
}

list_t *aux_get_bodies(aux_t *aux) { return aux->bodies; }

list_t *aux_get_constants(aux_t *aux) { return aux->constants; }

void aux_free(void *aux) {
  aux_t *aux_casted = (aux_t *)aux;
  if (aux_casted->constants != NULL) {
    list_free(aux_casted->constants);
  }

  list_free(aux_casted->bodies);
  free(aux_casted);
}
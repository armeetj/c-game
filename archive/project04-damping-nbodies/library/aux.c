#include "aux.h"

typedef struct aux  {
    double c; // constant (G, k, or gamma)
    body_t *body1;
    body_t *body2;
} aux_t;

aux_t* aux_init(double c, body_t *body1, body_t *body2) {
    aux_t *aux = malloc(sizeof(aux_t));
    aux->c = c;
    aux->body1 = body1;
    aux->body2 = body2;
    return aux;
}

body_t *get_body1(aux_t *aux) {
  return aux->body1;
}

body_t *get_body2(aux_t *aux) {
 return aux->body2; 
}

double get_constant(aux_t *aux) {
  return aux->c;
}

void aux_free(void *aux) {
  free(aux);
}
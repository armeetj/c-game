#ifndef __AUX_H__
#define __AUX_H__
#include "scene.h"
#include "body.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct aux aux_t;

aux_t *aux_init(double c, body_t *body1, body_t *body2);

double get_constant(aux_t *aux);

body_t *get_body1(aux_t *aux);

body_t *get_body2(aux_t *aux);

void aux_free(void *aux);

#endif // #ifndef __AUX_H__

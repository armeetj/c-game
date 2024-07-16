#ifndef __AUX_H__
#define __AUX_H__
#include "body.h"
#include "scene.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct aux aux_t;

aux_t *aux_init(list_t *constants, list_t *bodies);

list_t *aux_get_bodies(aux_t *aux);

list_t *aux_get_constants(aux_t *aux);

void aux_free(void *aux);

#endif // #ifndef __AUX_H__

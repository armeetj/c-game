#ifndef __COLLISION_PACKAGE_H__
#define __COLLISION_PACKAGE_H__

#include "body.h"
#include "forces.h"
#include "list.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct collision_package {
  body_t *body1;
  body_t *body2;
  collision_handler_t handler;
  void *aux;
  free_func_t freer;
} collision_package_t;

collision_package_t *collision_package_init(body_t *body1, body_t *body2,
                                            collision_handler_t handler,
                                            void *aux, free_func_t freer);

void collision_package_handle(collision_package_t *pkg);

void collision_package_free(void *pkg);

#endif // #ifndef __COLLISION_PACKAGE_H__
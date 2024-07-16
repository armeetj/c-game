#ifndef __FORCE_WRAPPER_H__
#define __FORCE_WRAPPER_H__

#include "forces.h"
#include "scene.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct force_wrapper force_wrapper_t;

/**
 * Initialize force_wrapper with a given force creator, aux, and freer
 *
 * @param force_creator
 * @param aux
 * @param freer
 * @return force_wrapper_t*
 */
force_wrapper_t *force_init(force_creator_t force_creator, void *aux,
                            free_func_t freer);

/**
 * Initialize force_wrapper with a given force creator, aux, freer, and bodies
 *
 * @param force_creator
 * @param aux
 * @param freer
 * @param bodies
 * @return force_wrapper_t*
 */
force_wrapper_t *force_init_with_bodies(force_creator_t force_creator,
                                        void *aux, free_func_t freer,
                                        list_t *bodies);

/**
 * Returns bodies associated with a force
 *
 * @param f the force_wrapper_t
 * @return list_t * of bodies associated.
 */
list_t *force_get_bodies(force_wrapper_t *f);

/**
 * Creates a force using the force creator
 * Passes in the aux value
 *
 * @param force
 * @return vector_t
 */
void force_create(force_wrapper_t *force);

/**
 * Get aux value stored in this force wrapper
 *
 * @param force pointer to instance
 * @return void*
 */
void *force_get_aux(force_wrapper_t *force);

/**
 * Replaces the force wrapper's aux value to the provided one
 *
 * @param force pointer to instance
 * @param aux new aux value
 */
void force_set_aux(force_wrapper_t *force, void *aux);

/**
 * Free all resources related to this force
 *
 * @param force
 */
void force_free(void *force);

void force_remove(force_wrapper_t *force);

bool force_is_removed(force_wrapper_t *force);

#endif // #ifndef __FORCE_WRAPPER_H__

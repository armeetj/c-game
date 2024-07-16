#ifndef __FORCE_WRAPPER_H__
#define __FORCE_WRAPPER_H__

#include "scene.h"
#include "forces.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct force_wrapper force_wrapper_t;

/**
 * Initialize force_wrapper with a given force creator, aux, and freer
 * 
 * @param force_creator 
 * @param aux 
 * @param freer 
 * @return force_wrapper_t* 
 */
force_wrapper_t *force_init(force_creator_t force_creator, void *aux, free_func_t freer);

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

#endif // #ifndef __FORCE_WRAPPER_H__

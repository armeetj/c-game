#ifndef __SCENE_H__
#define __SCENE_H__

#include "body.h"
#include "list.h"
#include <stdbool.h>

/**
 * A collection of bodies and force creators.
 * The scene automatically resizes to store
 * arbitrarily many bodies and force creators.
 */
typedef struct scene scene_t;

/**
 * A function which adds some forces or impulses to bodies,
 * e.g. from collisions, gravity, or spring forces.
 * Takes in an auxiliary value that can store parameters or state.
 */
typedef void (*force_creator_t)(void *aux);

/**
 * Allocates memory for an empty scene.
 * Makes a reasonable guess of the number of bodies to allocate space for.
 * Asserts that the required memory is successfully allocated.
 *
 * @return the new scene
 */
scene_t *scene_init(void);

/**
 * Releases memory allocated for a given scene
 * and all the bodies and force creators it contains.
 *
 * @param scene a pointer to a scene returned from scene_init()
 */
void scene_free(scene_t *scene);

/**
 * Gets the number of bodies in a given scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @return the number of bodies added with scene_add_body()
 */
size_t scene_bodies(scene_t *scene);

/**
 * Gets the body at a given index in a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 * @return a pointer to the body at the given index
 */
body_t *scene_get_body(scene_t *scene, size_t index);

/**
 * Adds a body to a scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param body a pointer to the body to add to the scene
 */
void scene_add_body(scene_t *scene, body_t *body);

/**
 * Removes and frees the body at a given index from a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 */
void scene_remove_body(scene_t *scene, size_t index);

/**
 * Get dev mode status
 * 
 * @param scene pointer to scene 
 * @return true in dev mode
 * @return false not in dev mode
 */
bool scene_get_dev_mode(scene_t *scene);

/**
 * Sets scene dev_mode
 * 
 * @param scene pointer to curr scene
 * @param dev_mode whether dev mode is on/off
 */
void scene_set_dev_mode(scene_t *scene, bool dev_mode);

/**
 * Adds a force creator to a scene,
 * to be invoked every time scene_tick() is called.
 * The auxiliary value is passed to the force creator each time it is called.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param forcer a force creator function
 * @param aux an auxiliary value to pass to forcer when it is called
 * @param freer if non-NULL, a function to call in order to free aux
 */
void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer);

/**
 * Executes a tick of a given scene over a small time interval.
 * This requires executing all the force creators
 * and then ticking each body (see body_tick()).
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param dt the time elapsed since the last tick, in seconds
 */
void scene_tick(scene_t *scene, double dt);

/**
 * Draws all the bodies in a given scene
 * 
 * @param scene Scene containing bodies to draw
 */
void scene_draw(scene_t *scene);

/**
 * Resets acceleration of all bodies in a scene
 * 
 * @param scene Scene containing bodies to draw
 */
void scene_accel_reset(scene_t *scene);

#endif // #ifndef __SCENE_H__

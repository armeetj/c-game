#ifndef __SCENE_H__
#define __SCENE_H__

#include "body.h"
#include "list.h"
#include <stdbool.h>

/**
 * A collection of bodies.
 * The scene automatically resizes to store arbitrarily many bodies.
 */
typedef struct scene scene_t;

/**
 * Allocates memory for an empty scene.
 * Makes a reasonable guess of the number of bodies to allocate space for.
 * Asserts that the required memory is successfully allocated.
 *
 * @return scene_t* the new scene
 */
scene_t *scene_init(void);

/**
 * Releases memory allocated for a given scene and all its bodies.
 *
 * @param scene a pointer to a scene returned from scene_init()
 */
void scene_free(scene_t *scene);

/**
 * Gets the number of bodies in a given scene
 *
 * @param scene pointer to provided scene
 * @return size_t number of bodies in provided scene
 */
size_t scene_bodies(scene_t *scene);

/**
 * Gets the body at a given index in a scene.
 * Asserts that the index is valid.
 *
 * @param scene pointer to provided scene
 * @param index index of body in the scene (starts at 0)
 * @return body_t* pointer to body at provided index
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
 * Executes a tick of a given scene over a small time interval.
 * This requires ticking each body in the scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param dt the time elapsed since the last tick, in seconds
 */
void scene_tick(scene_t *scene, double dt);

#endif // #ifndef __SCENE_H__

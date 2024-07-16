#ifndef __BODY_H__
#define __BODY_H__

#include "color.h"
#include "list.h"
#include "vector.h"

/**
 * A rigid body constrained to the plane.
 * Implemented as a polygon with uniform density.
 */
typedef struct body body_t;

/**
 * Allocates memory for a body with the given parameters.
 * The body is initially at rest.
 * Asserts that the required memory is allocated.
 *
 * @param shape a list of vectors describing the initial shape of the body
 * @param mass the mass of the body (if INFINITY, stops the body from moving).
 *   For now, the mass is unused. It will be needed next week for accelerations.
 * @param color the color of the body, used to draw it on the screen
 * @return body_t * a pointer to the newly allocated body
 */
body_t *body_init(list_t *shape, double mass, rgb_color_t color);

/**
 * Releases the memory allocated for a body.
 *
 * @param body a pointer to a body returned from body_init()
 */
void body_free(body_t *body);

/**
 * Gets the current shape of a body.
 * Returns a newly allocated vector list, which must be list_free()d.
 *
 * @param body a pointer to a body returned from body_init()
 * @return list_t* the polygon describing the body's current position
 */
list_t *body_get_shape(body_t *body);

/**
 * Set the current body shape.
 * Replaces the previous body shape.
 *
 * @param body
 * @param shape
 */
void body_set_shape(body_t *body, list_t *shape);

/**
 * Gets the current center of mass of a body.
 * While this could be calculated with polygon_centroid(), that becomes too slow
 * when this function is called thousands of times every tick.
 * Instead, the body should store its current centroid.
 *
 * @param body a pointer to a body returned from body_init()
 * @return vector_t the body's center of mass
 */
vector_t body_get_centroid(body_t *body);

/**
 * Gets the current position of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return vector_t the body's position vector
 */
vector_t body_get_position(body_t *body);

/**
 * Updates position of body
 *
 * @param body reference to body
 * @param pos new position vector
 */
void body_set_position(body_t *body, vector_t pos);

/**
 * Gets the current velocity of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return vector_t the body's velocity vector
 */
vector_t body_get_velocity(body_t *body);

/**
 * Updates velocity of body
 *
 * @param body reference to body
 * @param vel new velocity vector
 */
void body_set_velocity(body_t *body, vector_t vel);

/**
 * Gets the current acceleration of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return vector_t the body's acceleration vector
 */
vector_t body_get_acceleration(body_t *body);

/**
 * Updates acceleration of body
 *
 * @param body reference to body
 * @param acl new acceleration vector
 */
void body_set_acceleration(body_t *body, vector_t acl);

/**
 * Gets the display color of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return rgb_color_t the body's color, as an (R, G, B) tuple
 */
rgb_color_t body_get_color(body_t *body);

/**
 * Translates a body to a new position.
 * The position is specified by the position of the body's center of mass.
 *
 * @param body a pointer to a body returned from body_init()
 * @param x the body's new centroid
 */
void body_set_centroid(body_t *body, vector_t x);

/**
 * Changes a body's velocity (the time-derivative of its position).
 *
 * @param body a pointer to a body returned from body_init()
 * @param v the body's new velocity
 */
void body_set_velocity(body_t *body, vector_t v);

/**
 * Changes a body's orientation in the plane.
 * The body is rotated about its center of mass.
 * Note that the angle is *absolute*, not relative to the current orientation.
 *
 * @param body a pointer to a body returned from body_init()
 * @param angle the body's new angle in radians. Positive is counterclockwise.
 */
void body_set_rotation(body_t *body, double angle);

/**
 * Moves a body at its current velocity over a given time interval.
 * Next week, this will take forces and impulses on the body into account.
 *
 * @param body the body to tick
 * @param dt the number of seconds elapsed since the last tick
 */
void body_tick(body_t *body, double dt);

#endif // #ifndef __BODY_H__

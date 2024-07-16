#include "collision.h"
#include "list.h"
#include "vector.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Returns a list of lines perpendicular to the edges of both shapes
list_t *get_perpendicular_lines(list_t *shape1, list_t *shape2) {
  list_t *perpendicular_lines =
      list_init(list_size(shape1) + list_size(shape2), free);

  // Iterates through all the vertices in shape1
  for (size_t i = 0; i < list_size(shape1); i++) {
    vector_t v1 = *((vector_t *)list_get(shape1, i));
    vector_t v2 = *((vector_t *)list_get(shape1, (i + 1) % list_size(shape1)));

    // Calculates the vector from vertex 1 to vertex 2
    vector_t parallel_vec = vec_normalize(vec_subtract(v2, v1));
    vector_t perpendicular_vec = vec_normalize(vec_perpendicular(parallel_vec));
    vector_t *perp_vec = malloc(sizeof(vector_t));
    *perp_vec = perpendicular_vec;
    list_add(perpendicular_lines, perp_vec);
  }

  // Iterates through all the vertices in shape2
  for (size_t i = 0; i < list_size(shape2); i++) {
    vector_t v1 = *((vector_t *)list_get(shape2, i));
    vector_t v2 = *((vector_t *)list_get(shape2, (i + 1) % list_size(shape2)));

    // Calculates the vector from vertex 1 to vertex 2
    vector_t parallel_vec = vec_normalize(vec_subtract(v2, v1));
    vector_t perpendicular_vec = vec_normalize(vec_perpendicular(parallel_vec));
    vector_t *perp_vec = malloc(sizeof(vector_t));
    *perp_vec = perpendicular_vec;
    list_add(perpendicular_lines, perp_vec);
  }

  return perpendicular_lines;
}

/**
 * Returns a list containing the starting point and ending point
 * of the projection of 'shape' onto 'line.'
 */
vector_t project_shape(list_t *shape, vector_t line) {

  // Initializes the starting point and ending point as the first point in the
  // shape
  double min_length = vec_dot(*((vector_t *)list_get(shape, 0)), line);
  double max_length = vec_dot(*((vector_t *)list_get(shape, 0)), line);

  // Iterates through all the vertices in the shape
  for (size_t i = 0; i < list_size(shape); i++) {

    // Projects the point (vector from origin to the vertex) onto the line
    vector_t point = *((vector_t *)list_get(shape, i));
    double vec_len = vec_dot(point, line);

    // Tests if the projected vector is smaller than the starting point or
    // larger than ending point
    if (vec_len < min_length) {
      min_length = vec_len;
    } else if (vec_len > max_length) {
      max_length = vec_len;
    }
  }

  // Adds starting and ending point to a list and returns it
  vector_t endpoints = (vector_t){.x = min_length, .y = max_length};
  return endpoints;
}

/**
 * Determines if there exists some axis perpendicular to 'line'
 * that separates shape1 from shape2. True if such a line doesn't exist
 * (shapes intersect), false if a line exists (shapes do not intersect)
 */
bool intersect(vector_t line, list_t *shape1, list_t *shape2) {

  // Gets the starting and endoing point of the projected vector of shape onto
  // line
  vector_t shape1_endpoints = project_shape(shape1, line);
  vector_t shape2_endpoints = project_shape(shape2, line);

  double min1 = shape1_endpoints.x;
  double max1 = shape1_endpoints.y;
  double min2 = shape2_endpoints.x;
  double max2 = shape2_endpoints.y;

  if (min1 <= max2 && max1 >= min2) {
    return true;
  }
  return false;
}

collision_info_t find_collision(list_t *shape1, list_t *shape2) {
  list_t *perpendicular_lines = get_perpendicular_lines(shape1, shape2);
  collision_info_t collision_data;
  double smallest_overlap = INFINITY;
  vector_t collision_axis;
  double overlap = 0; // was -11
  for (size_t i = 0; i < list_size(perpendicular_lines); i++) {
    if (!intersect(*((vector_t *)list_get(perpendicular_lines, i)), shape1,
                   shape2)) {
      collision_data.collided = false;
      list_free(perpendicular_lines);
      return collision_data;
    } else {
      vector_t axis = *((vector_t *)list_get(perpendicular_lines, i));
      double min1 = project_shape(shape1, axis).x;
      double max1 = project_shape(shape1, axis).y;
      double min2 = project_shape(shape2, axis).x;
      double max2 = project_shape(shape2, axis).y;
      overlap = fmin(fabs(min1 - max2), fabs(max1 - min2));
      if (i == 0) {
        smallest_overlap = overlap;
        collision_axis = vec_normalize(axis);
      } else if (overlap < smallest_overlap) {
        smallest_overlap = overlap;
        collision_axis = vec_normalize(axis);
      }
    }
  }
  collision_data.collided = true;
  collision_data.axis = collision_axis;
  list_free(perpendicular_lines);
  return collision_data;
}

#include "../include/polygon.h"
#include "../include/test_util.h"
#include "../include/vec_list.h"
#include "../include/vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

double polygon_area(vec_list_t *polygon) {
  double area = 0.0;
  for (size_t i = 0; i <= vec_list_size(polygon) - 1; i++) {
    vector_t v1 = *vec_list_get(polygon, i % vec_list_size(polygon));
    vector_t v2 = *vec_list_get(polygon, (i + 1) % vec_list_size(polygon));
    area += 0.5 * (v2.x + v1.x) * (v2.y - v1.y);
  }
  return fabs(area);
}

vector_t polygon_centroid(vec_list_t *polygon) {
  double area = polygon_area(polygon);
  double x_coord = 0.0;
  double y_coord = 0.0;
  for (size_t i = 0; i <= vec_list_size(polygon) - 1; i++) {
    vector_t v1 = *vec_list_get(polygon, i % vec_list_size(polygon));
    vector_t v2 = *vec_list_get(polygon, (i + 1) % vec_list_size(polygon));
    x_coord += (v1.x + v2.x) * (v1.x * v2.y - v2.x * v1.y);
    y_coord += (v1.y + v2.y) * (v1.x * v2.y - v2.x * v1.y);
  }

  // formula said use 6
  x_coord /= 6 * area;
  y_coord /= 6 * area;

  vector_t newVec = {.x = x_coord, .y = y_coord};

  return newVec;
}

void polygon_translate(vec_list_t *polygon, vector_t translation) {
  for (size_t i = 0; i < vec_list_size(polygon); i++) {
    vector_t *v = vec_list_get(polygon, i);
    *v = vec_add(*v, translation);
  }
}

void polygon_rotate(vec_list_t *polygon, double angle, vector_t point) {
  for (size_t i = 0; i < vec_list_size(polygon); i++) {
    vector_t *original_vector = vec_list_get(polygon, i);
    vector_t temp_vector = {.x = original_vector->x - point.x,
                            .y = original_vector->y - point.y};
    vector_t rotate_origin = vec_rotate(temp_vector, angle);
    double x_coord = point.x + rotate_origin.x;
    double y_coord = point.y + rotate_origin.y;
    original_vector->x = x_coord;
    original_vector->y = y_coord;
  }
}

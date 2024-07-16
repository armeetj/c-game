#include "polygon.h"
#include "color.h"
#include "list.h"
#include "test_util.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

double polygon_area(list_t *polygon) {
  double area = 0.0;
  for (size_t i = 0; i <= list_size(polygon) - 1; i++) {
    vector_t v1 = *((vector_t *)list_get(polygon, i % list_size(polygon)));
    vector_t v2 =
        *((vector_t *)list_get(polygon, (i + 1) % list_size(polygon)));
    area += 0.5 * (v2.x + v1.x) * (v2.y - v1.y);
  }
  return fabs(area);
}

vector_t polygon_centroid(list_t *polygon) {
  double area = polygon_area(polygon);
  double x_coord = 0.0;
  double y_coord = 0.0;
  for (size_t i = 0; i <= list_size(polygon) - 1; i++) {
    vector_t v1 = *((vector_t *)list_get(polygon, i % list_size(polygon)));
    vector_t v2 =
        *((vector_t *)list_get(polygon, (i + 1) % list_size(polygon)));
    x_coord += (v1.x + v2.x) * (v1.x * v2.y - v2.x * v1.y);
    y_coord += (v1.y + v2.y) * (v1.x * v2.y - v2.x * v1.y);
  }

  // formula said use 6
  x_coord /= 6.0 * area;
  y_coord /= 6.0 * area;

  vector_t newVec = {.x = x_coord, .y = y_coord};

  return newVec;
}

void polygon_translate(list_t *polygon, vector_t translation) {
  for (size_t i = 0; i < list_size(polygon); i++) {
    vector_t *v = list_get(polygon, i);
    *v = vec_add(*v, translation);
  }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
  for (size_t i = 0; i < list_size(polygon); i++) {
    vector_t *original_vector = list_get(polygon, i);
    vector_t temp_vector = {.x = original_vector->x - point.x,
                            .y = original_vector->y - point.y};
    vector_t rotate_origin = vec_rotate(temp_vector, angle);
    double x_coord = point.x + rotate_origin.x;
    double y_coord = point.y + rotate_origin.y;
    original_vector->x = x_coord;
    original_vector->y = y_coord;
  }
}

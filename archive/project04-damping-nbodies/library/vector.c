#include "vector.h"
#include <math.h>

const vector_t VEC_ZERO = {.x = 0.0, .y = 0.0};

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t newVec = {.x = v1.x + v2.x, .y = v1.y + v2.y};
  return newVec;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  return vec_add(v1, vec_negate(v2));
}

vector_t vec_negate(vector_t v) { return vec_multiply(-1, v); }

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t newVec = {.x = scalar * v.x, .y = scalar * v.y};
  return newVec;
}


double vec_dot(vector_t v1, vector_t v2) {
  return (v1.x * v2.x) + (v1.y * v2.y);
}

double vec_cross(vector_t v1, vector_t v2) {
  return (v1.x * v2.y) - (v1.y * v2.x);
}

vector_t vec_rotate(vector_t v, double angle) {
  vector_t newVec = {.x = v.x * cos(angle) - v.y * sin(angle),
                     .y = v.x * sin(angle) + v.y * cos(angle)};
  return newVec;
}

double vec_norm (vector_t v) {
  return sqrt(vec_dot(v, v));
}

double vec_dist(vector_t v1, vector_t v2) {
  return vec_norm(vec_subtract(v1, v2));
}

vector_t vec_normalize(vector_t v) {
  return vec_multiply(1.0 / vec_norm(v), v);
}
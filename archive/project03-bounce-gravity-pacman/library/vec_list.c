#include "vector.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct vec_list {
  vector_t **vector_list;
  size_t size;
  size_t capacity;
} vec_list_t;

vec_list_t *vec_list_init(size_t initial_size) {
  vec_list_t *vectors = malloc(sizeof(vec_list_t));
  vectors->vector_list = malloc(initial_size * sizeof(vector_t *));
  assert(vectors != NULL);
  assert(vectors->vector_list != NULL);
  vectors->size = 0;
  vectors->capacity = initial_size;
  return vectors;
}

void vec_list_free(vec_list_t *list) {
  for (size_t i = 0; i < list->size; i++) {
    free(list->vector_list[i]);
  }
  free(list->vector_list);
  free(list);
}

size_t vec_list_size(vec_list_t *list) { return list->size; }

vector_t *vec_list_get(vec_list_t *list, size_t index) {
  assert(index < list->size);
  return list->vector_list[index];
}

void vec_list_add(vec_list_t *list, vector_t *value) {
  assert(value != NULL);
  assert(list->size < list->capacity);
  list->vector_list[list->size] = value;
  list->size++;
}

vector_t *vec_list_remove(vec_list_t *list) {
  assert(list->size > 0);
  list->size--;
  return list->vector_list[list->size];
}

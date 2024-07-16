#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

const size_t RESIZE_MULTIPLIER = 2;
const size_t DEFAULT_CAPACITY = 10;

typedef struct list {
  void **data;
  free_func_t freer;
  size_t size;
  size_t capacity;
} list_t;

/**
 * @brief
 *
 * @param initial_capacity
 * @return list_t*
 */
list_t *list_init(size_t initial_capacity, free_func_t freer) {
  list_t *list = malloc(sizeof(list_t));
  list->data = malloc(sizeof(void *) * initial_capacity);
  list->size = 0;
  list->freer = freer;
  list->capacity = initial_capacity;
  // asserts if correct size was allocated
  assert(list->data);
  return list;
}

void list_resize(list_t *list) {
  size_t new_capacity = (list->capacity * RESIZE_MULTIPLIER) + 1;
  list->data = realloc(list->data, sizeof(void *) * new_capacity);
  list->capacity = new_capacity;
  assert(list->data);
}

void list_add(list_t *list, void *element) {
  assert(element != NULL);
  if (list->size == list->capacity) {
    list_resize(list);
  }
  list->data[list->size] = element;
  list->size++;
}

void *list_remove(list_t *list, size_t index) {
  // assert valid index
  assert(index >= 0 && index < list->size);
  void *temp_data = list->data[index];

  for (size_t idx = index; idx < list->size - 1; idx++) {
    list->data[idx] = list->data[idx + 1];
  }

  list->size--;
  return temp_data;
}

void *list_get(list_t *list, size_t index) {
  // assert valid index
  assert(index >= 0 && index < list->size);
  return list->data[index];
}

void list_free(void *list) {
  list_t *casted_list = (list_t *) list;
  if (casted_list->freer != NULL) {
    for (size_t i = 0; i < casted_list->size; i++) {
      casted_list->freer(list_get(casted_list, i));
    }
  }
  free(casted_list->data);
  free(list);
}

size_t list_size(list_t *list) { return list->size; }

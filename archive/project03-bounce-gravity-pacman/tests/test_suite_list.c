#include "list.h"
#include "test_util.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_empty_list_size() {
  list_t *l = list_init(0, free);
  assert(list_size(l) == 0);
  list_free(l);
}

void test_list_add_1_string() {
  list_t *l = list_init(1, free);
  char *temps = malloc(sizeof(char) * 3);
  strcpy(temps, "hi");
  list_add(l, temps);
  assert(!strcmp((char *)list_get(l, 0), temps));
  list_free(l);
}

void test_resize() {
  list_t *l = list_init(0, free);
  assert(list_size(l) == 0);
  int32_t *temp1 = malloc(sizeof(int32_t));
  int32_t *temp2 = malloc(sizeof(int32_t));
  int32_t *temp3 = malloc(sizeof(int32_t));
  int32_t *temp4 = malloc(sizeof(int32_t));
  *temp1 = 1;
  *temp2 = 2;
  *temp3 = 3;
  *temp4 = 4;
  list_add(l, temp1);
  list_add(l, temp2);
  list_add(l, temp3);
  list_add(l, temp4);
  assert(list_size(l) == 4);
  list_free(l);
}

void test_get() {
  list_t *l = list_init(1000, free);
  int32_t *temp_int = malloc(sizeof(int32_t));
  *temp_int = 3;
  list_add(l, temp_int);
  assert(temp_int == (int32_t *)list_get(l, 0));
  list_free(l);
}

// TODO: verify this works
void test_remove() {
  list_t *l = list_init(1, free);
  size_t size_prev = list_size(l);
  int32_t *temp_int = malloc(sizeof(int32_t));
  *temp_int = 3;
  list_add(l, temp_int);
  void *removed_element = list_remove(l, 0);
  assert((int32_t *)removed_element ==
         temp_int);       // assert the correct value was removed
  assert(*temp_int == 3); // assert value hasn't changed
  assert(list_size(l) ==
         size_prev - 1); // assert the size decreased appropriately
  list_free(l);
  free(removed_element);
  free(temp_int);
}
int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_empty_list_size)
  DO_TEST(test_resize)
  DO_TEST(test_get)
  DO_TEST(test_list_add_1_string)

  puts("list_test PASS");
}

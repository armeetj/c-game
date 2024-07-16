#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "forces.h"
#include "test_util.h"

double vec_norm(vector_t v) { return sqrt(vec_dot(v, v)); }

vector_t vec_normalize(vector_t v) {
  return vec_multiply(1.0 / vec_norm(v), v);
}

list_t *make_shape() {
  list_t *shape = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){-1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+1, +1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){-1, +1};
  list_add(shape, v);
  return shape;
}

void test_spring_dampening() {
  const double M = 10;
  const double K = 2;
  const double A = 3;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  scene_t *scene = scene_init();
  body_t *mass = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_centroid(mass, (vector_t){A, 0});
  scene_add_body(scene, mass);
  body_t *anchor = body_init(make_shape(), INFINITY, (rgb_color_t){0, 0, 0});
  scene_add_body(scene, anchor);
  create_spring(scene, K, mass, anchor);
  double max_amplitude = A;
  for (int i = 0; i < STEPS; i++) {
    if (i != 0 && fabs(1 - fabs(cos(sqrt(K / M) * i * DT))) < 1e-5) {
      double curr_amplitude = body_get_centroid(mass).x;
      assert(curr_amplitude <= max_amplitude);
      max_amplitude = curr_amplitude;
    }
    scene_tick(scene, DT);
  }
  scene_free(scene);
}

// Tests that two springs in parallel, with spring constant K,
// have a spring constant equal to 2 * K

void test_spring_parallel() {
  const double M = 10;
  const double K = 2;
  const double A = 3;
  const double DT = 1e-6;
  const int STEPS = 1000;
  scene_t *scene = scene_init();
  body_t *mass1 = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_centroid(mass1, (vector_t){A, 0});
  scene_add_body(scene, mass1);
  body_t *anchor1 = body_init(make_shape(), INFINITY, (rgb_color_t){0, 0, 0});
  scene_add_body(scene, anchor1);
  create_spring(scene, 2 * K, mass1, anchor1);

  body_t *mass2 = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_centroid(mass2, (vector_t){A, 0});
  scene_add_body(scene, mass2);
  body_t *anchor2 = body_init(make_shape(), INFINITY, (rgb_color_t){0, 0, 0});
  body_t *anchor3 = body_init(make_shape(), INFINITY, (rgb_color_t){0, 0, 0});
  scene_add_body(scene, anchor2);
  scene_add_body(scene, anchor3);
  create_spring(scene, K, mass2, anchor2);
  create_spring(scene, K, mass2, anchor3);

  for (int i = 0; i < STEPS; i++) {
    assert(vec_isclose(body_get_centroid(mass1), body_get_centroid(mass2)));
    scene_tick(scene, DT);
  }
  scene_free(scene);
}

// Tests that two springs with identical K constants attached symmetrically to
// an object will not move the object.
void test_nonmoving_spring() {
  const double M = 10;
  const double K = 2.0;
  const double A = 3;
  const double DT = 1e-6;
  const int STEPS = 1000;
  scene_t *scene = scene_init();

  body_t *mass = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_centroid(mass, (vector_t){0, 0});
  scene_add_body(scene, mass);

  body_t *anchor1 = body_init(make_shape(), INFINITY, (rgb_color_t){0, 0, 0});
  body_set_centroid(anchor1, (vector_t){-A, 0});
  scene_add_body(scene, anchor1);
  create_spring(scene, K, mass, anchor1);

  body_t *anchor2 = body_init(make_shape(), INFINITY, (rgb_color_t){0, 0, 0});
  body_set_centroid(anchor2, (vector_t){A, 0});
  scene_add_body(scene, anchor2);
  create_spring(scene, K, mass, anchor2);

  vector_t zero_vec = (vector_t){.x = 0, .y = 0};
  for (int i = 0; i < STEPS; i++) {
    assert(vec_isclose(body_get_centroid(mass), zero_vec));
    scene_tick(scene, DT);
  }
  scene_free(scene);
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_spring_parallel)
  DO_TEST(test_nonmoving_spring)
  DO_TEST(test_spring_dampening)

  puts("student_tests PASS");

  return 0;
}

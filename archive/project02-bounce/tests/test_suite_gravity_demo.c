#include "../demo/gravity.c"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <vector.h>

void test_shape_struct()
{
    for (size_t num_points = 1; num_points < 1000; num_points++)
    {
        shape_t *new_shape = malloc(sizeof(shape_t));
        *new_shape = (shape_t){
            .visible = false,
            .pts = make_star(CIRCLE_RAD, vector(SPAWN.x, SPAWN.y), 5 + num_points),
            .pos = vector(SPAWN.x, SPAWN.y),
            .vel = vector(VEL_X, 0),
            .rot = 0.0,
            .rot_velocity = 0.01,
            .time_since_collision = 0.0,
            .elasticity = randRange(0.8, 0.99),
            .color = (rgb_color_t){.r = (float)rand() / (float)RAND_MAX, .g = (float)rand() / (float)RAND_MAX, .b = (float)rand() / (float)RAND_MAX}};
        free(new_shape);
    }
    assert(true);
}

// TODO: to implement
void test_small_vs_large()
{
    assert(true);
}

// TODO: implement html test
void test_bg_black()
{
    assert(true);
}

// TODO: check shape velocities obey gravity
void test_arc_path()
{
    assert(true);
}


int main(int argc, char *argv[]) {
  DO_TEST(test_small_vs_large)
  puts("gravity_demo_test PASS");
}

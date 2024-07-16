#include "collision_package.h"
#include "collision.h"

collision_package_t *collision_package_init(body_t *body1, body_t *body2,
                                            collision_handler_t handler,
                                            void *aux, free_func_t freer) {
  collision_package_t *package = malloc(sizeof(collision_package_t));
  package->body1 = body1;
  package->body2 = body2;
  package->handler = handler;
  package->aux = aux;
  package->freer = freer;
  return package;
}

void collision_package_handle(collision_package_t *pkg) {
  body_t *body1 = pkg->body1;
  body_t *body2 = pkg->body2;
  list_t *shape1 = body_get_shape(body1);
  list_t *shape2 = body_get_shape(body2);
  if (find_collision(shape1, shape2).collided) {
    vector_t axis = find_collision(shape1, shape2).axis;
    pkg->handler(body1, body2, axis, pkg->aux);
  }
  list_free(shape1);
  list_free(shape2);
}

void collision_package_free(void *pkg) {
  collision_package_t *pkg_casted = (collision_package_t *)pkg;
  if (pkg_casted->freer != NULL) {
    pkg_casted->freer(pkg_casted->aux);
  }
  free(pkg_casted);
}
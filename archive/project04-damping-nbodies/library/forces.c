#include "forces.h"
#include "aux.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

const double MIN_DIST = 30;

void gravity_force_creator(void *aux)
{
    aux_t *aux_casted = (aux_t *)aux;
    double G = get_constant(aux_casted);
    body_t *body1 = get_body1(aux_casted);
    body_t *body2 = get_body2(aux_casted);
    vector_t v1 = body_get_position(body1);
    vector_t v2 = body_get_position(body2);
    double dist = vec_dist(v2, v1);
    if (dist >= MIN_DIST) {
        vector_t r = vec_subtract(v2, v1);
        vector_t r_hat = vec_multiply(1.0 / vec_norm(r), r);
        double numerator = G * body_get_mass(body1) * body_get_mass(body2);
        double denominator = vec_norm(r) * vec_norm(r);
        double g_scal = numerator / denominator;
        vector_t g_vec = vec_multiply(g_scal, r_hat);
        body_add_force(body1, g_vec);
        body_add_force(body2, vec_negate(g_vec));
    }
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1, body_t *body2)
{
    force_creator_t g_creator = gravity_force_creator;
    aux_t *g_aux = aux_init(G, body1, body2);
    scene_add_force_creator(scene, g_creator, g_aux, aux_free);
}

void spring_force_creator(void *aux)
{
    aux_t *aux_casted = (aux_t *) aux;
    double k = get_constant(aux_casted);
    body_t *body1 = get_body1(aux_casted);
    body_t *body2 = get_body2(aux_casted);
    vector_t distance = vec_subtract(body_get_position(body2), body_get_position(body1));
    vector_t s_vec = vec_multiply(-k, distance);
    body_add_force(body2, s_vec);
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2)
{
    force_creator_t s_creator = spring_force_creator;
    aux_t *s_aux = aux_init(k, body1, body2);
    scene_add_force_creator(scene, s_creator, s_aux, aux_free);
}

void drag_force_creator(void *aux) // TODO: add aux to arguments
{
    // unpackage aux
    aux_t *aux_casted = (aux_t *) aux;
    double gamma = get_constant(aux_casted);
    body_t *body = get_body1(aux_casted);

    vector_t vel = body_get_velocity(body);
    vector_t d_vec = vec_multiply(-gamma, vel);
    body_add_force(body, d_vec);
}

void create_drag(scene_t *scene, double gamma, body_t *body)
{
    force_creator_t d_creator = drag_force_creator;
    aux_t *d_aux = aux_init(gamma, body, NULL);
    scene_add_force_creator(scene, d_creator, d_aux, aux_free);
}


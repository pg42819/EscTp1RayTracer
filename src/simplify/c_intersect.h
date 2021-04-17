#pragma once

#ifndef ISPC
#include <stdbool.h>
#endif
#include "c_vec.h"

/**
 * Simple C struct for a triangle
 */
typedef struct tri {
    c_vec3f vertices[3];
    c_vec3f normals[3]; // why should a single triangle have 3 normals?
//    vec3f normal;

    c_vec3f ka;
    c_vec3f kd;
    c_vec3f ks;
    c_vec3f ke;
    float Ns;
    int prim_id; // id of the triangle (face) in the object (geom)
    int geom_id; // id of the object (geom) in the scene
} c_triangle;


#if defined(__cplusplus) || defined(ISPC)
extern "C" {
#endif

extern bool c_intersect_triangle(c_vec3f orig, c_vec3f dir, c_triangle *triangle,
                                 float *t, float *u, float *v);

#if defined(ISPC)
}
extern "C" {
#endif

extern void c_intersect_triangles(c_triangle *triangles, int num_triangles,
                                  c_vec3f origin, c_vec3f direction,
                                  float *near_t_ptr, float *u_ptr, float *v_ptr,
                                  int *geom_id, int *prim_id);

#if defined(__cplusplus) || defined(ISPC)
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern c_vec3f new_vec3f(float x, float y, float z);

/**
 * Vector dot product
 * C = A.B = AxBx + AyBy + AzBz
 */
extern double vec3f_dot(c_vec3f a, c_vec3f b);

/**
 * Vector cross product
 * formula: C = A x B
 *   Cx = AyBz − AzBy
 *   Cy = AzBx − AxBz
 *   Cz = AxBy − AyBx
 */
extern c_vec3f vec3f_cross(c_vec3f a, c_vec3f b);

/**
 * Vector addition
 * formula: C = A + B
 *   Cx = Ax + Bx
 *   Cy = Ay + By
 *   Cz = Az + Bz
 */
extern c_vec3f vec3f_add(c_vec3f a, c_vec3f b);

/**
 * Vector subtraction
 * formula: C = A - B
 *   Cx = Ax - Bx
 *   Cy = Ay - By
 *   Cz = Az - Bz
 */
extern c_vec3f vec3f_subtract(c_vec3f a, c_vec3f b);

/**
 * Vector multiply by scalar float
 * formula: C = nA
 *   Cx = Ax * n
 *   Cy = Ay * n
 *   Cz = Az * n
 */
extern c_vec3f vec3f_multiply_scalar(c_vec3f a, float n);

/**
 * Vector divide by scalar float
 * formula: C = A / n
 *   Cx = Ax / n
 *   Cy = Ay / n
 *   Cz = Az / n
 */
extern c_vec3f vec3f_divide_scalar(c_vec3f a, float n);

/**
 * Vector magnitude
 * formula ||A|| = sqrt(A . A)
 */
extern float vec3f_magnitude(c_vec3f a);

/**
 * Vector normalize (convert to unit vector in same direction)
 * formula N = A / ||A||
 */
extern c_vec3f vec3f_normalize(c_vec3f a);

#ifdef __cplusplus
}
#endif
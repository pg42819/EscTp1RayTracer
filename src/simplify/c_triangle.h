#pragma once
#include <stdbool.h>
#include "c_vec.h"

/**
 * Simple C struct for a triangle
 */
typedef struct {
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


extern bool c_intersect_triangle(c_vec3f orig, c_vec3f dir, c_triangle *triangle,
                                 float *t, float *u, float *v);
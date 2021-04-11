#include <float.h>
#include <stdbool.h>
//#include "c_triangle.h"
//
//bool NOOP_c_intersect_triangle(c_vec3f orig, c_vec3f dir, c_triangle *triangle,
//                          float *t, float *u, float *v)
//{
//    c_vec3f edge1, edge2, tvec, pvec, qvec;
//    double determinant, inverse_determinant;
//    c_vec3f vert0 = triangle->vertices[0];
//    c_vec3f vert1 = triangle->vertices[1];
//    c_vec3f vert2 = triangle->vertices[2];
//
//    /* find vectors for two edges sharing vert0 */
//    //    edge1 = vert1 - vert0;
//    edge1 = vec3f_subtract(vert1, vert0);
//    //    edge2 = vert2 - vert0;
//    edge2 = vec3f_subtract(vert2, vert0);
//
//    /* begin calculating determinant - also used to calculate U parameter */
//    /* pvec is normal to the ray direction and one edge */
//    pvec = vec3f_cross(dir, edge2);
//    determinant = vec3f_dot(edge1, pvec);
//
//    /* if determinant is near zero, ray lies in plane of triangle */
//    if (determinant > -DBL_EPSILON && determinant < DBL_EPSILON) {
//        return false;
//    }
//
//    inverse_determinant = 1.0f / determinant;
//
//    /* calculate distance from vert0 to ray origin */
//    // tvec = orig - vert0;
//    tvec = vec3f_subtract(orig, vert0);
//
//    /* calculate U parameter and test bounds */
//    double u2 = vec3f_dot(tvec, pvec) * inverse_determinant;
//    if (u2 < DBL_EPSILON || u2 > 1.0f) {
//        return false;
//    }
//
//    /* prepare to test V parameter */
//    qvec = vec3f_cross(tvec, edge1);
//
//    /* calculate V parameter and test bounds */
//    double v2 = vec3f_dot(dir, qvec) * inverse_determinant;
//    if (v2 < DBL_EPSILON  || u2 + v2 > 1.0f) {
//        return false;
//    }
//
//    /* calculate t, ray intersects triangle */
//    double t2 = vec3f_dot(edge2, qvec) * inverse_determinant;
//    if (t2 < DBL_EPSILON) {
//       return false;
//    }
//    if (t2 >= *t) {
//        return false;
//    }
//    *t = t2;
//    *u = u2;
//    *v = v2;
//
//    return true;
//}

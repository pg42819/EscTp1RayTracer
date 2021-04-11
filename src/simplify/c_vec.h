#pragma once

/**
 * 3-float vector - equivalent of vec3 in vec.h but for use in ispc or pure c not only c++
 */
typedef struct {
    float x;
    float y;
    float z;
} c_vec3f;

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
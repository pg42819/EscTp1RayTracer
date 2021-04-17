#ifndef ISPC
#include <math.h>
#endif
#include "c_vec.h"

c_vec3f new_vec3f(float x, float y, float z)
{
    c_vec3f new_vec; // create on stack
    new_vec.x = x;
    new_vec.y = y;
    new_vec.z = z;
    return new_vec;
}

//
// Standard Vector operations on vectors of 3 floats

/**
 * Vector dot product
 * C = A.B = AxBx + AyBy + AzBz
 */
double vec3f_dot(c_vec3f a, c_vec3f b) {
    double dot_product = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    return dot_product;
}

/**
 * Vector cross product
 * formula: C = A x B
 *   Cx = AyBz − AzBy
 *   Cy = AzBx − AxBz
 *   Cz = AxBy − AyBx
 */
c_vec3f vec3f_cross(c_vec3f a, c_vec3f b) {
    c_vec3f cross_product;
    cross_product.x = (a.y * b.z) - (a.z * b.y);
    cross_product.y = (a.z * b.x) - (a.x * b.z);
    cross_product.z = (a.x * b.y) - (a.y * b.x);
    return cross_product;
}

/**
 * Vector addition
 * formula: C = A + B
 *   Cx = Ax + Bx
 *   Cy = Ay + By
 *   Cz = Az + Bz
 */
c_vec3f vec3f_add(c_vec3f a, c_vec3f b) {
    c_vec3f sum;
    sum.x = a.x + b.x;
    sum.y = a.y + b.y;
    sum.z = a.z + b.z;
    return sum;
}

/**
 * Vector subtraction
 * formula: C = A - B
 *   Cx = Ax - Bx
 *   Cy = Ay - By
 *   Cz = Az - Bz
 */
c_vec3f vec3f_subtract(c_vec3f a, c_vec3f b) {
    c_vec3f diff;
    diff.x = a.x - b.x;
    diff.y = a.y - b.y;
    diff.z = a.z - b.z;
    return diff;
}

/**
 * Vector multiply by scalar float
 * formula: C = nA
 *   Cx = Ax * n
 *   Cy = Ay * n
 *   Cz = Az * n
 */
c_vec3f vec3f_multiply_scalar(c_vec3f a,  float n) {
    c_vec3f product;
    product.x = a.x * n;
    product.y = a.y * n;
    product.z = a.z * n;
    return product;
}

/**
 * Vector divide by scalar float
 * formula: C = A / n
 *   Cx = Ax / n
 *   Cy = Ay / n
 *   Cz = Az / n
 */
c_vec3f vec3f_divide_scalar(c_vec3f a,  float n) {
    c_vec3f quotient;
    quotient.x = a.x / n;
    quotient.y = a.y / n;
    quotient.z = a.z / n;
    return quotient;
}

/**
 * Vector magnitude
 * formula ||A|| = sqrt(A . A)
 */
float vec3f_magnitude(c_vec3f a)
{
    float magnitude = sqrtf(vec3f_dot(a, a));
    return magnitude;
}

/**
 * Vector normalize (convert to unit vector in same direction)
 * formula N = A / ||A||
 */
c_vec3f vec3f_normalize(c_vec3f a) {
    float magnitude = vec3f_magnitude(a);
    c_vec3f normalized = vec3f_divide_scalar(a, magnitude);
    return normalized;
}

/**
 * Inverse of a vector
 * formula v = v1 ^ (-1)
 * v.x = 1 / v1.x
 * v.y = 1 / v1.y
 * v.z = 1 / v1.z
 */
c_vec3f inverseVector(c_vec3f a) {
    c_vec3f result = new_vec3f(1 / a.x, 1 / a.y, 1 / a.z);

    return result;
}
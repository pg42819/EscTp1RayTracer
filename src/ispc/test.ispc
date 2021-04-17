//
// Example ispc file to test compiler
//
#include "ispc_helpers.h"


void assert_vec3f(c_vec3f vec, uniform float x, uniform float y, uniform float z)
{
   print("Asserting: % % %", x, y, z);
   assert(vec.x == x);
   assert(vec.y == y);
   assert(vec.z == z);
   print(" YEP!\n");
}

void assert_scalar(double scalar, uniform float expected)
{
    print("Asserting: %, got %", expected, scalar);
    print(" YEP!\n");
}

export void test_vectors_ispc()
{
    varying c_vec3f v1 = new_vec3f(1.f, 0.f, 1.f);
    varying c_vec3f v2 = new_vec3f(1.f, 2.f, 3.f);

    print("[1 0 1] . [1 2 3] = 4   ");
    assert_scalar(vec3f_dot(v1,v2), 4.f);

    print("[1 0 1] x [1 2 3] = [-2 -2 2] ");
    assert_vec3f(vec3f_cross(v1, v2), -2.f, -2.f, 2.f);

    print("[1 0 1] + [1 2 3] = [2 2 4]   ");
    assert_vec3f(vec3f_add(v1, v2), 2.f, 2.f, 4.f);

    print("[1 0 1] - [1 2 3] = [0 -2 -2] ");
    assert_vec3f(vec3f_subtract(v1, v2), 0.f, -2.f, -2.f);
}
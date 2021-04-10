#pragma once
#include <math.h>
#include <string.h>

/**
 * 3-float vector - equivalent of vec3 in vec.h but for use in ispc or pure c not only c++
 */
typedef struct {
    float x;
    float y;
    float z;
} vec3f;

vec3f new_vec3f(float x, float y, float z)
{
    vec3f new_vec; // create on stack
    new_vec.x = x;
    new_vec.y = y;
    new_vec.z = z;
    return new_vec;
}

/**
 * Simple C struct for a triangle
 */
typedef struct {
    vec3f vertices[3];
    vec3f normals[3]; // why should a single triangle have 3 normals?
//    vec3f normal;

    vec3f ka;
    vec3f kd;
    vec3f ks;
    vec3f ke;
    float Ns;
    int prim_id; // id of the triangle (face) in the object (geom)
    int geom_id; // id of the object (geom) in the scene
} c_triangle;


//template <typename T> vec4<T> operator/(const vec4<T> &v, const T &scalar) {
//  vec4<T> r;
//  for (int i = 0; i < 4; i++) {
//    r.data[i] = v.data[i] / scalar;
//  }
//  return r;
//}

//template <typename T> T dot(const vec4<T> &v0, const vec4<T> &v1) {
//  T sum2 = 0;
//  for (int i = 0; i < 4; i++) {
//    sum2 += (v0.data[i] * v1.data[i]);
//  }
//  return sum2;
//}
//
//template <typename T> vec4<T> normalize(const vec4<T> &v) {
//  return v / dot(v, v);
//}
//
//template <typename T> T dot(const vec3<T> &v0, const vec3<T> &v1) {
//  T sum = 0;
//  for (int i = 0; i < 3; i++) {
//    sum += (v0.data[i] * v1.data[i]);
//  }
//  return sum;
//}
//
//template <typename T> vec3<T> cross(const vec3<T> &v1, const vec3<T> &v2) {
//  vec3<T> dest;
//  dest[0] = v1[1] * v2[2] - v1[2] * v2[1];
//  dest[1] = v1[2] * v2[0] - v1[0] * v2[2];
//  dest[2] = v1[0] * v2[1] - v1[1] * v2[0];
//  return dest;
//}
//
//template <typename T> vec3<T> operator+(const vec3<T> &v1, const vec3<T> &v2) {
//  return vec3<T>(v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]);
//}
//
//template <typename T> vec3<T> operator-(const vec3<T> &v1, const vec3<T> &v2) {
//  return vec3<T>(v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]);
//}
//
//template <typename T> vec3<T> operator/(const vec3<T> &v, const T &scalar) {
//  vec3<T> r;
//  for (int i = 0; i < 3; i++) {
//    r.data[i] = v.data[i] / scalar;
//  }
//  return r;
//}
//
//template <typename T> vec3<T> operator*(const vec3<T> &v, const T &scalar) {
//  vec3<T> r;
//  for (int i = 0; i < 3; i++) {
//    r.data[i] = v.data[i] * scalar;
//  }
//  return r;
//}
//
//template <typename T> vec3<T> normalize(const vec3<T> &v) {
//  return v / sqrt(dot(v, v));
//}
//
//template <typename T> T length(const vec3<T> &v) { return sqrt(dot(v, v)); }
//
//} // namespace tracer
//
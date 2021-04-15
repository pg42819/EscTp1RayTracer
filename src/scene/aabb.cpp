#pragma once
#include <stdlib.h>
#include <cmath>
#include "scene.h"
#include "../math/vec.h"
#include "../simplify/c_vec.h"
#include "aabb.h"

inline float ffmin(float a, float b) { return a < b ? a : b;}
inline float ffmax(float a, float b) { return a > b ? a : b;}

aabb aabb::surrounding_box(aabb box0, aabb box1) {
    c_vec3f small;
    small.x = ffmin(box0.min().x, box1.min().x);
    small.y = ffmin(box0.min().y, box1.min().y);
    small.z = ffmin(box0.min().z, box1.min().z);
    
    c_vec3f big;
    big.x = ffmax(box0.max().x, box1.max().x);
    big.y = ffmax(box0.max().y, box1.max().y);
    big.z = ffmax(box0.max().z, box1.max().z);

    return aabb(small, big);

}

void aabb::expandToIncludePoint(const c_vec3f &v) {
    c_vec3f new_min, new_max;
    new_min.x = ffmin(_min.x, v.x);
    new_min.y = ffmin(_min.y, v.y);
    new_min.z = ffmin(_min.z, v.z);

    new_max.x = ffmax(_max.x, v.x);
    new_max.y = ffmax(_max.y, v.y);
    new_max.z = ffmax(_max.z, v.z);

    _min = new_min;
    _max = new_max;
    
    _extent = vec3f_subtract(_max,_min);
}

void aabb::expandToIncludeBox(const aabb& box) {
    c_vec3f new_min, new_max;
    new_min.x = ffmin(_min.x, box._min.x);
    new_min.y = ffmin(_min.y, box._min.y);
    new_min.z = ffmin(_min.z, box._min.z);

    new_max.x = ffmax(_max.x, box._max.x);
    new_max.y = ffmax(_max.y, box._max.y);
    new_max.z = ffmax(_max.z, box._max.z);

    _min = new_min;
    _max = new_max;
    
    _extent = vec3f_subtract(_max,_min);
}

c_vec3f aabb::getCenter() {

    c_vec3f vec = vec3f_add(_min, _max);

    vec = vec3f_multiply_scalar(vec, (float) 0.5);

    return vec;
}

bool aabb::intersect(const c_vec3f &ori, const c_vec3f &dir, float* tnear, float* tfar) {

    c_vec3f inv_d = inverseVector(dir);

    float tmin = (_min.x - ori.x) * inv_d.x;
    float tmax = (_max.x - ori.x) * inv_d.x;

    if (tmin > tmax) {
        std::swap(tmin, tmax);
    }

    float tymin = (_min.y - ori.y) * inv_d.y;
    float tymax = (_max.y - ori.y) * inv_d.y;

    if (tymin > tymax) {
        std::swap(tymin, tymax);
    }

    if ((tmin > tymax) || (tymin > tmax)) {
        return false;
    }

    tmin = std::fmax(tymin, tmin);
    tmax = std::fmin(tymax, tmax);

    float tzmin = (_min.z - ori.z) * inv_d.z;
    float tzmax = (_max.z - ori.z) * inv_d.z;

    if (tzmin > tzmax) {
        std::swap(tzmin, tzmax);
    }

    if ((tmin > tzmax) || (tzmin > tmax)) {
        return false;
    }

    tmin = std::fmax(tzmin, tmin);
    tmax = std::fmin(tzmax, tmax);

    *tnear = tmin;
    *tfar = tmax;

    return true;
}

float aabb::surfaceArea() {
    return (float) ((_extent.x * _extent.z) + (_extent.x * _extent.y) + (_extent.y * _extent.z));
}
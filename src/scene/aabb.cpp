#pragma once
#include <stdlib.h>
#include "scene.h"
#include "../math/vec.h"
#include "../simplify/c_vec.h"
#include "aabb.h"

inline float ffmin(float a, float b) { return a < b ? a : b;}
inline float ffmax(float a, float b) { return a > b ? a : b;}

aabb surrounding_box(aabb box0, aabb box1) {
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

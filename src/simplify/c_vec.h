#pragma once

/**
 * 3-float vector - equivalent of vec3 in vec.h but for use in ispc or pure c not only c++
 */
typedef struct c_vec {
    float x;
    float y;
    float z;
} c_vec3f;

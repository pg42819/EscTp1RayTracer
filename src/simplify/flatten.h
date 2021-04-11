#pragma once
#include "../scene/scene.h"
#include "../math/vec.h"

/**
 * Flatten the geometry objects into a continous array of triangles
 * @param SceneMesh
 */
extern void flatten_scene(tracer::scene &SceneMesh);

extern c_vec3f cpp_vec_to_c_vec(const tracer::vec3<float> &cpp_vec);
#pragma once
#include "../scene/scene.h"
#include "c_vec.h"
#include "trace_ispc.h"
#include "../math/vec.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Flatten the geometry objects into a continous array of triangles
 * @param SceneMesh
 */
extern void flatten_scene(tracer::scene &SceneMesh, ispc::ispc_triangle **flat_triangles, int *num_triangles);

extern c_vec3f cpp_vec_to_c_vec(const tracer::vec3<float> &cpp_vec);
#ifdef __cplusplus
}
#endif

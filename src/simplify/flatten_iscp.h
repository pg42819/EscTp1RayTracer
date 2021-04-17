#include <src/scene/scene.h>
// ispc compiler generated
#include "trace_ispc.h"

/**
 * Enhances the SceneMesh with a flat array of triangles representing every triangle in
 * every object in the original geometry
 * @param SceneMesh 
 */
extern void flatten_scene_ispc(tracer::scene &SceneMesh, ispc::ispc_triangle **flat_triangles, int *num_triangles);

#include <src/scene/scene.h>
// ispc compiler generated
#include "trace_ispc.h"

/**
 * Extracts a flat array of triangles and light-source triangles from scenemesh
 */
extern void flatten_scene_ispc(tracer::scene &SceneMesh,
                        ispc::ispc_triangle **flat_triangles, int *num_triangles,
                        ispc::ispc_light **flat_lights, int *num_lights,
                        ispc::ispc_triangle **light_faces, int *num_light_faces);

extern void new_ispc_cam(ispc::ispc_cam *ispc_cam,
                         tracer::vec3<float> lookfrom, tracer::vec3<float> lookat, tracer::vec3<float> vup,
                         float vfov, float aspect);

#include <src/scene/scene.h>
// ispc compiler generated
#include "trace_ispc.h"


/**
 * Holds flat vectors of data for triangles and lights for passing to ISPC
 */
struct FlatScene {
    std::vector<ispc::ispc_triangle> triangles;
    std::vector<ispc::ispc_triangle> light_faces;
    std::vector<ispc::ispc_light> lights;
};

/**
 * Extracts a flat array of triangles and light-source triangles from scenemesh
 */
extern void flatten_scene_ispc(tracer::scene &SceneMesh, FlatScene &flat_scene, int debug);

extern void new_ispc_cam(ispc::ispc_cam *ispc_cam,
                         tracer::vec3<float> lookfrom, tracer::vec3<float> lookat, tracer::vec3<float> vup,
                         float vfov, float aspect);

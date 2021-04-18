#include <algorithm>
#include <iostream>
#include <src/scene/scene.h>
#include <src/scene/camera.h>
#include "../debug.h"
#include "../math/vec.h"
//#include "c_triangle.h"
// ispc compiler generated
#include "trace_ispc.h"
#include "flatten_iscp.h"
//
// Convert between CPP and C representations
//

// comparison function to be used to sort by x coordinate of center
bool compare_triangle_x_iscp(const ispc::ispc_triangle &t1, const ispc::ispc_triangle &t2)
{
    // compare the x coord of the centroid of the triangle (average of the x coords of each corner)
    int x = 0;
    float t1_center_x = (t1.vertices[0][x] + t1.vertices[1][x] + t1.vertices[2][x]) / 3;
    float t2_center_x = (t2.vertices[0][x] + t2.vertices[1][x] + t2.vertices[2][x]) / 3;
    return t1_center_x < t2_center_x;
}

/**
 * Enhances the SceneMesh with a flat array of triangles representing every triangle in
 * every object in the original geometry
 * @param SceneMesh 
 */
//void flatten_scene_ispc(tracer::scene &SceneMesh,
//                        ispc::ispc_triangle *flat_triangles, int *num_triangles,
//                        ispc::ispc_light *flat_lights, int *num_lights,
//                        ispc::ispc_triangle *light_faces, int *num_light_faces,
//                        int debug)
void flatten_scene_ispc(tracer::scene &SceneMesh, FlatScene &flat_scene, int debug)
{
   for (auto geom_id = 0; geom_id < SceneMesh.geometry.size(); geom_id++) {
        // indexes of light face (triangles) in this object if this object is a light source
        std::vector<int> light_source_face_indexes;

        // geom is an object in the scene, mat is the material it is covered in (for light reflection)
        tracer::scene::Geometry &geom = SceneMesh.geometry[geom_id];
        tracer::scene::Material &mat = geom.object_material;

        int has_normals = !geom.normals.empty();
        int is_light = 0;

        // SceneMesh.light_source is a vector of _indexes_ in the geom (object) vector for the scene,
        // where the object at each index is a light source. So search to see if _this_ geom is a light source
        if (std::find(SceneMesh.light_sources.begin(), SceneMesh.light_sources.end(), geom_id) !=
            SceneMesh.light_sources.end()) {
            is_light = 1;
        }

        for (auto f = 0; f < geom.face_index.size(); f++) {
            // face represents a face in the object (triangle), where each face is the 3 indices
            // in the objects big vertex array that form the corners of a triangle.
            tracer::vec3<unsigned int> &face = geom.face_index[f];
            // create a new simple C struct triangle
            ispc::ispc_triangle triangle;
            triangle.geom_id = geom_id; // id of the object
            triangle.prim_id = f; // id of the triangle
            triangle.has_normals = has_normals;
            triangle.is_light = is_light;

            // store the material properties in the triangle
            // use array copying to get a copy of the material in each triangle
            // (TODO consider an index into a separate material array to save memory)
            std::copy(std::begin(mat.ka.data), std::end(mat.ka.data), std::begin(triangle.ka));
            std::copy(std::begin(mat.kd.data), std::end(mat.kd.data), std::begin(triangle.kd));
            std::copy(std::begin(mat.ks.data), std::end(mat.ks.data), std::begin(triangle.ks));
            std::copy(std::begin(mat.ke.data), std::end(mat.ke.data), std::begin(triangle.ke));
            triangle.Ns = mat.Ns;

            for (auto v = 0; v < 3; v++) {
                int corner_idx = face[v];
                triangle.vertices[v][0] = geom.vertex[corner_idx].x;
                triangle.vertices[v][1] = geom.vertex[corner_idx].y;
                triangle.vertices[v][2] = geom.vertex[corner_idx].z;
                if (has_normals) {
                    triangle.normals[v][0] = geom.normals[corner_idx].x;
                    triangle.normals[v][1] = geom.normals[corner_idx].y;
                    triangle.normals[v][2] = geom.normals[corner_idx].z;
                }
            }

            flat_scene.triangles.push_back(triangle);

            if (is_light) {
                // keep a separate list of light-source triangles so that we
                // can index into it after sorting the main list
                flat_scene.light_faces.push_back(triangle);
                // store the index in the global light-source-triangle array on the
                // light_source record for this geom
                light_source_face_indexes.push_back(flat_scene.light_faces.size() - 1); // index into light of this source
            }
        } // each face (triangle)

        if (is_light) {
            ispc::ispc_light light;
            light.geom_id = geom_id; // not really used
            // store the indexes as a flat array for use in ispc
            light.light_faces = light_source_face_indexes.data();
            light.num_light_faces = light_source_face_indexes.size();
            flat_scene.lights.push_back(light);
        }
    } // each geom

    // sort by x coordinate - makes for easher bvh tree building later
    std::sort(flat_scene.triangles.begin(), flat_scene.triangles.end(), compare_triangle_x_iscp);
}


/**
 * Create a plain c struct for the ispc use of the camera
 */
void new_ispc_cam(ispc::ispc_cam *ispc_cam,
                  tracer::vec3<float> lookfrom, tracer::vec3<float> lookat, tracer::vec3<float> vup,
                  float vfov, float aspect)
{
    for (auto i = 0; i < 3; i++) {
        ispc_cam->lookfrom[i] = lookfrom.data[i];
        ispc_cam->lookat[i] = lookat.data[i];
        ispc_cam->vup[i] = vup.data[i];
    }
    ispc_cam->vfov = vfov;
    ispc_cam->aspect = aspect;
}
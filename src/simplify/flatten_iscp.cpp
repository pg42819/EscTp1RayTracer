#include <algorithm>
#include <src/scene/scene.h>
#include "flatten.h"
#include "c_vec.h"
//#include "c_triangle.h"
// ispc compiler generated
#include "trace_ispc.h"

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
void flatten_scene_ispc(tracer::scene &SceneMesh, ispc::ispc_triangle **flat_triangles, int *num_triangles)
{
    std::vector<ispc::ispc_triangle> triangles;
    for (auto i = 0; i < SceneMesh.geometry.size(); i++) {
        // geom is an object in the scene
        tracer::scene::Geometry &geom = SceneMesh.geometry[i];
        int has_normals = !geom.normals.empty();
        for (auto f = 0; f < geom.face_index.size(); f++) {
            // face represents a face in the object (triangle), where each face is the 3 indices
            // in the objects big vertex array that form the corners of a triangle.
            tracer::vec3<unsigned int> &face = geom.face_index[f];
            // create a new simple C struct triangle
            ispc::ispc_triangle triangle;
            triangle.geom_id = i; // id of the object
            triangle.prim_id = f; // id of the triangle
            triangle.has_normals = has_normals;
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
            // TODO do we need material?
            triangles.push_back(triangle);
        }
    }

    // sort by x coordinate - makes for easher bvh tree building later
    std::sort(triangles.begin(), triangles.end(), compare_triangle_x_iscp);

    // attach the triangles to the scene as simple C array and size
    *flat_triangles = triangles.data();
    *num_triangles = triangles.size();
}




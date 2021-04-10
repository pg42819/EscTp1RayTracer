#include <src/scene/scene.h>
#include "flatten.h"

//
// Flatten scheme geometry into triangles
//

/**
 * Enhances the SceneMesh with a flat array of triangles representing every triangle in
 * every object in the original geometry
 * @param SceneMesh 
 */
void flatten_scene(tracer::scene &SceneMesh)
{
    std::vector<c_triangle> c_triangles;
    for (auto i = 0; i < SceneMesh.geometry.size(); i++) {
        // geom is an object in the scene
        tracer::scene::Geometry &geom = SceneMesh.geometry[i];
        for (auto f = 0; f < geom.face_index.size(); f++) {
            // face represents a face in the object, where each face is the 3 indices in the objects big
            // vertex set that form the corners of a triangle.
            tracer::vec3<unsigned int> &face = geom.face_index[f];
            // create a new simple C struct triangle
            c_triangle triangle;
            triangle.geom_id = i; // id of the object
            triangle.prim_id = f; // id of the triangle
            for (auto v = 0; v < 3; v++) {
                vec3f corner;
                corner.x = geom.vertex[face[v]].x;
                corner.y = geom.vertex[face[v]].y;
                corner.z = geom.vertex[face[v]].z;
                triangle.vertices[v] = corner;
            }
            // TODO do we need normals - or material?
            c_triangles.push_back(triangle);
        }
    }

    // attach the triangles to the scene as simple C array and size
    SceneMesh.c_triangles = c_triangles.data();
    SceneMesh.num_triangles = c_triangles.size();
}




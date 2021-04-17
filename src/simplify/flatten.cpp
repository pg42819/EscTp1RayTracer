#include <algorithm>
#include <iostream>
#include <src/scene/scene.h>
#include "flatten.h"
#include "c_vec.h"
#include "c_triangle.h"

//
// Convert between CPP and C representations
//

c_vec3f cpp_vec_to_c_vec(const tracer::vec3<float> &cpp_vec)
{
    c_vec3f c_vec;
    c_vec.x = cpp_vec.x;
    c_vec.y = cpp_vec.y;
    c_vec.z = cpp_vec.z;
    return c_vec;
}

// comparison function to be used to sort by x coordinate of point
bool leftMostTriangle(const c_triangle &t1, const c_triangle &t2)
{
    // compare the x coord of the first vertex (note we don't know that the first
    // vertex is the leftmost itself but close enough)
    return t1.vertices[0].x < t2.vertices[0].x;
}

//comparison function to sort triangles by y axis.
bool triangleYComparison(const c_triangle &t1, const c_triangle &t2)
{
    // compare the x coord of the first vertex (note we don't know that the first
    // vertex is the leftmost itself but close enough)
    return t1.vertices[0].y < t2.vertices[0].y;
}

//comparison function to sort triangles by z axis.
bool triangleZComparison(const c_triangle &t1, const c_triangle &t2)
{
    // compare the x coord of the first vertex (note we don't know that the first
    // vertex is the leftmost itself but close enough)
    return t1.vertices[0].z < t2.vertices[0].z;
}

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
                c_vec3f corner;
                corner.x = geom.vertex[face[v]].x;
                corner.y = geom.vertex[face[v]].y;
                corner.z = geom.vertex[face[v]].z;
                triangle.vertices[v] = corner;

                std::cout << "O X é: " << corner.x << "\n";
                std::cout << "O Y é: " << corner.y << "\n";
                std::cout << "O Z é: " << corner.z << "\n";
            }
            // TODO do we need normals - or material?
            c_triangles.push_back(triangle);
        }
    }

    // sort by x coordinate - makes for easher bvh tree building later
    std::sort(c_triangles.begin(), c_triangles.end(), leftMostTriangle);
    // attach the triangles to the scene as simple C array and size
    SceneMesh.c_triangles = c_triangles.data();
    SceneMesh.num_triangles = c_triangles.size();
}




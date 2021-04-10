#include <stdlib.h>
#include "scene.h"
#include "../math/vec.h"
#include "bvh.h"

void flattenGeom(tracer::scene &SceneMesh) {
    std::vector<triangle> triangles;

    for(auto i = 0; i < SceneMesh.geometry.size(); i++) {
        for(auto j = 0; j < SceneMesh.geometry[i].face_index.size(); j++) {
            triangle t;
            t.primitiveID = i;
            t.objID = j;

            tracer::vec3<unsigned int> &face = c.x = SceneMesh.geometry[i].face_index[j];
            for(auto k = 0; k < 3; k++) {
                tracer::vec3<float> c;

                c.x = SceneMesh.geometry[i].vertex[face[k]].x;
                c.y = SceneMesh.geometry[i].vertex[face[k]].y;
                c.z = SceneMesh.geometry[i].vertex[face[k]].z;

                t.vertices[k] = c;
            }

            triangles.push_back(t);
        }
    }

    //TODO
    //ADD TRIANGLE VECTOR TO SCENEMESH
}
#pragma once

#include "../math/vec.h"
#include <vector>
#include "../simplify/c_triangle.h"

namespace tracer {
struct scene {

  struct Material {
    tracer::vec3<float> ka;
    tracer::vec3<float> kd;
    tracer::vec3<float> ks;
    tracer::vec3<float> ke;
    float Ns;
    bool lightsource{false};
  };

  struct Geometry {
    std::vector<tracer::vec3<float> > vertex;
    std::vector<tracer::vec3<float> > normals;
    std::vector<tracer::vec4<float> > vertex_color; // not used
    std::vector<tracer::vec2<float> > uv; // not used
    // face_index is a vector of faces - ie triangles.
    // Each face_index is a vector of 3 indices into the vertex vector defining
    // the three vertices that form a specific triangle
    std::vector<tracer::vec3<unsigned int> > face_index; //  of vertices for triangle
    std::vector<Material> face_material;
    Material object_material;
    unsigned int geomID;
  };

  // before ispc convert this into a flat array of vertices
  std::vector<Geometry> geometry;
  std::vector<size_t> light_sources;

  // array of c structures for simple triangles to represent geometry
  c_triangle *c_triangles;
  int num_triangles = 0;
};
} // namespace tracer

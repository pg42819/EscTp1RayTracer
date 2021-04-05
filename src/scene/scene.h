#pragma once

#include "../math/vec.h"
#include <vector>

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
    std::vector<tracer::vec3<unsigned int> > face_index; // order of vertices for triangle
    std::vector<Material> face_material;
    Material object_material;
    unsigned int geomID;
  };

  // before ispc convert this into a flat array of vertices
  std::vector<Geometry> geometry;
  std::vector<size_t> light_sources;
};
} // namespace tracer

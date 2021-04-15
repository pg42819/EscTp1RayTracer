#include <stdlib.h>
#include "scene.h"
#include "../math/vec.h"
#include "bvh.h"
#include <iostream>
#include <random>

bvh_node new_bvhNode(aabb &box, uint32_t start, uint32_t count) {
  bvh_node n;

  n.bbox = box;
  n.start = start;
  n.primitive_count = count;
  n.left = NULL;
  n.right = NULL;

  return n;
}

 
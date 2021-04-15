#pragma once

#include <math.h>
#include <string.h>
#include "../math/vec.h"
#include "scene.h"
#include "aabb.h"


typedef struct bvh_node {
    //! The bounding box of the node.
    aabb bbox;

    //! The index of the first primitive.
    uint32_t start;

    //! The number of primitives in this node.
    uint32_t primitive_count;

    bvh_node *left, *right;
} bvh_node;

extern bvh_node new_bvhNode(aabb &box, uint32_t start, uint32_t count, bool leaf);
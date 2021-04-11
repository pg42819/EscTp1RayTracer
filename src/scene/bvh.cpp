#include <stdlib.h>
#include "scene.h"
#include "../math/vec.h"
#include "bvh.h"
#include <iostream>
#include <random>

static std::mt19937 random_number_engine(time(0));

static float randomFloat01()
  {
    return (rand() / (RAND_MAX + 1.0));
    std::uniform_real_distribution<float> distribution(0.0, 1.0);
    return distribution(random_number_engine);
  }

static int boxCompareX(const void* a, const void* b) {
    aabb box_left, box_right;

    Hittable *ah = *(Hittable**) a;
    Hittable *bh = *(Hittable**) b;

    if(!ah->boundingBox(0,0,box_left) || !bh->boundingBox(0,0,box_right)) {
        std::cerr << "No bounding box in BVHNode constructor\n";
    }

    if(box_left.min().x - box_right.min().x < 0.0f)
        return -1;
    else
        return 1;
}

bvh_node::bvh_node(Hittable **l, int n, float time0, float time1) {
    
    qsort(l, n, sizeof(Hittable*), boxCompareX);

    if(n == 1) {
        left = right = l[0];
    } else if(n == 2) {
        left = l[0];
        right = l[1];
    } else {
        left = new bvh_node(l, n/2, time0, time1);
        right = new bvh_node(l+n/2, n - n/2, time0, time1);
    }

    aabb box_left, box_right;

    if (!left->boundingBox(time0, time1, box_left) || !right->boundingBox(time0, time1, box_right))
    {
      std::cerr << "No bounding box in BVHNode constructor\n";
    }

    box = surrounding_box(box_left, box_right);

}

bool bvh_node::hit(const c_vec3f &ori, const c_vec3f &dir, float t_min, float t_max, Hit& h) const
  {
    if (box.hit(ori, dir, t_min, t_max))
    {
      Hit left_hit, right_hit;
      bool hit_left = left->hit(ori, dir, t_min, t_max, left_hit);
      bool hit_right = right->hit(ori, dir, t_min, t_max, right_hit);

      if (hit_left && hit_right)
      {
        if (left_hit.t < right_hit.t)
        {
          h = left_hit;
        }
        else
        {
          h = right_hit;
        }
        return true;
      }
      else if (hit_left)
      {
        h = left_hit;
        return true;
      }
      else if (hit_right)
      {
        h = right_hit;
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      return false;
    }
}

bool bvh_node::boundingBox(float t0, float t1, aabb& b) const
  {
    b = box;
    return true;
  }

 
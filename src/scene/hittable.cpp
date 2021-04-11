#include "hittable.h"
#include "aabb.h"

void HittableList::add(Hittable* hittable) {
    hittables_.push_back(hittable);
}

bool HittableList::hit(const c_vec3f &ori, const c_vec3f &dir, float t_min, float t_max, Hit& h) const
  {
    Hit temp_hit;
    bool hit_anything = false;
    double closest = t_max;
    for (auto i = 0; i < hittables_.size(); ++i)
    {
      if (hittables_[i]->hit(ori, dir, t_min, closest, temp_hit))
      {
        hit_anything = true;
        closest = temp_hit.t;
        h = temp_hit;
      }
    }
    return hit_anything;
  }

  bool HittableList::boundingBox(float t0, float t1, aabb& box) const
  {
    if (hittables_.size() < 1)
    {
      return false;
    }

    aabb temp_box;
    if (hittables_[0]->boundingBox(t0, t1, temp_box))
    {
      return false;
    }
    else
    {
      box = temp_box;
    }

    for (auto i = 0; i < hittables_.size(); ++i)
    {
      if (hittables_[0]->boundingBox(t0, t1, temp_box))
      {
        box = surrounding_box(box, temp_box);
      }
      else
      {
        return false;
      }
    }
    return true;
  }
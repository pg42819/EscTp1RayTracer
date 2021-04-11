#ifndef HITTABLE_H
#define HITTABLE_H

#include "helpful.h"
#include "aabb.h"
#include "scene.h"
#include "hittable_list.h"

using std::shared_ptr;

class material;

struct hit_record {
    tracer::vec3<float> p;
    tracer::vec3<float> normal;
    shared_ptr<material> mat_prt;
    float t;
    float u;
    float v;
    bool front_face;

    inline void set_face_normal(const tracer::vec3<float> &ori, const tracer::vec3<float> &dir, const tracer::vec3<float> &outwardsNormal) {
        front_face = dot(dir, outwardsNormal) < 0;

        normal = front_face ? outwardsNormal : -outwardsNormal;
    }
};

class hittable {
    public:
        virtual bool hit(const tracer::vec3<float> &ori, const tracer::vec3<float> &dir, float t_min, float t_max, hit_record &rec) const = 0;
        virtual bool bounding_box(float t0, float t1, aabb &outputBox) const = 0;
};

bool hittable_list::bounding_box(float t0, float t1, aabb &box)
{
    if (list_size < 1)
        return false;
    aabb temp_box;
    bool first_true = list[0]->bounding_box(t0, t1, temp_box);
    if (!first_true)
        return false;
    else
        box = temp_box;
    for (int i = 1; i < list_size; i++)
    {
        if (list[i]->bounding_box(t0, t1, temp_box))
        {
            box = surrounding_box(box, temp_box);
        }
        else
            return false;
    }
    return true;
}


#endif
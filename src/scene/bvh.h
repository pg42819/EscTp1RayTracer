#pragma once

#include <math.h>
#include <string.h>
#include "../math/vec.h"
#include "scene.h"
#include "hittable.h"
#include "aabb.h"


class bvh_node : public Hittable {
    public:
        bvh_node() {}
        bvh_node(Hittable **l, int n, float time0, float time1);

        virtual bool hit(const c_vec3f &ori, const c_vec3f &dir, float t_min, float t_max, Hit& h) const;
        virtual bool boundingBox(float t0, float t1, aabb &box) const;

    private:
        Hittable *left, *right;
        aabb box;
};

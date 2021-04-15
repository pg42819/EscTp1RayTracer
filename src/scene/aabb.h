#pragma once
#include <stdlib.h>
#include "scene.h"
#include "../math/vec.h"
#include "../simplify/c_vec.h"

extern float ffmin(float a, float b);
extern float ffmax(float a, float b);

class aabb {
    public:

        aabb() {}
        aabb(const c_vec3f &a, const c_vec3f &b) {_min = a; _max = b; _extent = vec3f_subtract(_max, _min);}
        aabb(const c_vec3f &p) {aabb(p,p);}

        c_vec3f min() const {return _min; }
        c_vec3f max() const {return _max; }

        virtual aabb surrounding_box(aabb box0, aabb box1);

        virtual void expandToIncludePoint(const c_vec3f &v);

        virtual void expandToIncludeBox(const aabb &box);

        virtual c_vec3f getCenter();

        virtual bool intersect(const c_vec3f &ori, const c_vec3f &dir, float* tnear, float* tfar);

        virtual float surfaceArea();

    public:
        c_vec3f _min;
        c_vec3f _max;
        c_vec3f _extent; //difference between min and max points of the bounding box
};
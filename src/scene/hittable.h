#pragma once

//#include "helpful.h"
#include "aabb.h"
#include "scene.h"
#include "../simplify/c_vec.h"

using std::shared_ptr;

class material;

struct Hit {
    float t;
    c_vec3f p;
    c_vec3f n;
    //vec2 uv
    //Material *mat;
};

class Hittable {
    public:
        virtual bool hit(const c_vec3f &ori, const c_vec3f &dir, float t_min, float t_max, Hit& h) const = 0;
        virtual bool boundingBox(float t0, float t1, aabb &box) const = 0;
};

class HittableList : public Hittable {
    public:
        HittableList() {}
        HittableList(std::vector<Hittable*> hittables) : hittables_(hittables) {}

        void add(Hittable* hittable);

        virtual bool hit(const c_vec3f &ori, const c_vec3f &dir, float t_min, float t_max, Hit &h) const;
        virtual bool boundingBox(float t0, float t1, aabb &box) const;

    private:
    std::vector<Hittable*> hittables_; 
};
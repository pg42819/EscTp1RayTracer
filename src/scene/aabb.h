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
        aabb(const c_vec3f &a, const c_vec3f &b) {_min = a; _max = b;}

        c_vec3f min() const {return _min; }
        c_vec3f max() const {return _max; }

        inline bool hit(const c_vec3f &ori, const c_vec3f &dir, float tmin, float tmax) const{
            for (int a = 0; a < 3; a++)
                {
                    switch(a) {
                        float invD;
                        float t0, t1;
                        case 0:
                            invD = 1.0f / dir.x;
                            t0 = (min().x - ori.x) * invD;
                            t1 = (max().x - ori.x) * invD;
                            if (invD < 0.0f)
                                std::swap(t0, t1);
                            tmax = t1 < tmax ? t1 : tmax;   //F is the minimum value of the two end points
                            tmin = t0 > tmin ? t0 : tmin;   //f is the maximum value of the two starting points
                            if (tmax <= tmin)       //F <= f
                                return false;
                            break;
                        case 1:
                            invD = 1.0f / dir.y;
                            t0 = (min().y - ori.y) * invD;
                            t1 = (max().y - ori.y) * invD;
                            if (invD < 0.0f)
                                std::swap(t0, t1);
                            tmax = t1 < tmax ? t1 : tmax;   //F is the minimum value of the two end points
                            tmin = t0 > tmin ? t0 : tmin;   //f is the maximum value of the two starting points
                            if (tmax <= tmin)       //F <= f
                                return false;
                            break;
                        case 2:
                            invD = 1.0f / dir.z;
                            t0 = (min().z - ori.z) * invD;
                            t1 = (max().z - ori.z) * invD;
                            if (invD < 0.0f)
                                std::swap(t0, t1);
                            tmax = t1 < tmax ? t1 : tmax;   //F is the minimum value of the two end points
                            tmin = t0 > tmin ? t0 : tmin;   //f is the maximum value of the two starting points
                            if (tmax <= tmin)       //F <= f
                                return false;
                            break;
                    }
                }
            return true;
        }

        float area() {
            auto a = _max.x - _min.x;
            auto b = _max.y - _min.y;
            auto c = _max.z - _min.z;

            return 2 * (a*b + b*c + c*a);
        }

        int longest_axis() {
            auto a = _max.x - _min.x;
            auto b = _max.y - _min.y;
            auto c = _max.z - _min.z;

            if (a > b && a > c)
                return 0;
            else if (b > c)
                return 1;
            else
                return 2;
        }

    public:
        c_vec3f _min;
        c_vec3f _max;
};

extern aabb surrounding_box(aabb box0, aabb box1);

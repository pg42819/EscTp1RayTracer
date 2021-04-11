#include <stdlib.h>
#include "scene.h"
#include "../math/vec.h"
#include "../simplify/c_vec.h"

inline float ffmin(float a, float b) { return a < b ? a : b;}
inline float ffmax(float a, float b) { return a > b ? a : b;}

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
                        case 0:
                            float invD = 1.0f / dir.x;
                            float t0 = (min().x - ori.x) * invD;
                            float t1 = (max().x - ori.x) * invD;
                            if (invD < 0.0f)
                                std::swap(t0, t1);
                            tmax = t1 < tmax ? t1 : tmax;   //F is the minimum value of the two end points
                            tmin = t0 > tmin ? t0 : tmin;   //f is the maximum value of the two starting points
                            if (tmax <= tmin)       //F <= f
                                return false;
                            break;
                        case 1:
                            float invD = 1.0f / dir.y;
                            float t0 = (min().y - ori.y) * invD;
                            float t1 = (max().y - ori.y) * invD;
                            if (invD < 0.0f)
                                std::swap(t0, t1);
                            tmax = t1 < tmax ? t1 : tmax;   //F is the minimum value of the two end points
                            tmin = t0 > tmin ? t0 : tmin;   //f is the maximum value of the two starting points
                            if (tmax <= tmin)       //F <= f
                                return false;
                            break;
                        case 2:
                            float invD = 1.0f / dir.z;
                            float t0 = (min().z - ori.z) * invD;
                            float t1 = (max().z - ori.z) * invD;
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

aabb surrounding_box(aabb box0, aabb box1) {
    c_vec3f small;
    small.x = ffmin(box0.min().x, box1.min().x);
    small.y = ffmin(box0.min().y, box1.min().y);
    small.z = ffmin(box0.min().z, box1.min().z);
    
    c_vec3f big;
    big.x = ffmax(box0.max().x, box1.max().x);
    big.y = ffmax(box0.max().y, box1.max().y);
    big.z = ffmax(box0.max().z, box1.max().z);

    return aabb(small, big);

}

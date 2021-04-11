#include <stdlib.h>
#include "scene.h"
#include "../math/vec.h"
#include "helpful.h"

class aabb {
    public:

        aabb() {}
        aabb(const tracer::vec3<float> &a, const tracer::vec3<float> &b) {_min = a; _max = b;}
        
        tracer::vec3<float> min() {return _min; }
        tracer::vec3<float> max() {return _max; }

        bool hit(const tracer::vec3<float> &ori, const tracer::vec3<float> &dir, float tmin, float tmax) {
            for (int a = 0; a < 3; a++)
                {
                    float invD = 1.0f / dir[a];
                    float t0 = (min()[a] - ori[a]) * invD;
                    float t1 = (max()[a] - ori[a]) * invD;
                    if (invD < 0.0f)
                        std::swap(t0, t1);
                    tmax = t1 < tmax ? t1 : tmax;   //F is the minimum value of the two end points
                    tmin = t0 > tmin ? t0 : tmin;   //f is the maximum value of the two starting points
                    if (tmax <= tmin)       //F <= f
                        return false;
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
        tracer::vec3<float> _min;
        tracer::vec3<float> _max;
};

aabb surrounding_box(aabb box0, aabb box1) {
    tracer::vec3<float> small(ffmin(box0.min().x, box1.min().x),
                              ffmin(box0.min().y, box1.min().y),
                              ffmin(box0.min().z, box1.min().z));
    
    tracer::vec3<float> big(ffmax(box0.max().x, box1.max().x),
                            ffmax(box0.max().y, box1.max().y),
                            ffmax(box0.max().z, box1.max().z));

    return aabb(small, big);

}

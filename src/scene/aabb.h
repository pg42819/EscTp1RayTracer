#include <stdlib.h>
#include "scene.h"
#include "../math/vec.h"
#include "bvh.h"

namespace aabb {
    tracer::vec3<float> _min;
    tracer::vec3<float> _max;
    
    tracer::vec3<float> min() {return _min; }
    tracer::vec3<float> max() {return _max; }

    float ffmin(float a, float b) {
        return a < b ? a : b;
    }

    float ffmax(float a, float b) {
        return a > b ? a : b;
    }

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

}
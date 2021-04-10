#include <math.h>
#include <string.h>
#include "../math/vec.h"
#include "scene.h"


typedef struct triangle
{
    tracer::vec3<float> vertices[3];
    tracer::vec3<float> normal;

    int primitiveID;
    int objID;
} triangle;

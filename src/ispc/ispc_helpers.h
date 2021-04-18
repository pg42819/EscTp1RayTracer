#pragma once
// No math.h for ISPC :(
#include "../debug.h"
#define M_PI 3.1415926535f
#define EPSILON 0.0001f

// Note: not documented but some constants can found in source at:
//
// E.g.: UINT32_MAX  - max 32bit unsigned int (useful for random)
/**
 * 3-float vec3tor - equivalent of vec3 in vec3.h but for use in ispc or pure c not only c++
 */
typedef float<3> vec3;

// Note we don't use vec3 in here because we want this to be reusable in cpp
struct ispc_triangle {
    float vertices[3][3];
    float normals[3][3];
    int prim_id;
    int geom_id;
    int has_normals;
    int is_light;
    // material info: repeated across triangles in the same object for performance
    float ka[3];  // reflection rate (ambience)
    float kd[3];  // surface color (diffuse)
    float ks[3];  // specular intensity
    float ke[3];  // emission intensity
    float Ns;     // specular index
};

// ispc float<3> version for calculations
struct material {
    vec3 ka;  // reflection rate (ambience)
    vec3 kd;  // surface color (diffuse)
    vec3 ks;  // specular intensity
    vec3 ke;  // emission intensity
    float Ns;  // specular index
};

void get_material(ispc_triangle &triangle, material &mat) {
    vec3 ka = { triangle.ka[0], triangle.ka[1], triangle.ka[2] };
    vec3 kd = { triangle.kd[0], triangle.kd[1], triangle.kd[2] };
    vec3 ks = { triangle.ks[0], triangle.ks[1], triangle.ks[2] };
    vec3 ke = { triangle.ke[0], triangle.ke[1], triangle.ke[2] };
    mat.ka = ka;
    mat.kd = kd;
    mat.ks = ks;
    mat.ke = ke;
    mat.Ns = triangle.Ns;
}

struct ispc_light {
    int geom_id;
    int *light_faces; // array of indexes in the triangle array of the light source triangles
    int num_light_faces;
};

// structure to share the camera state with
struct ispc_cam {
    float lookfrom[3];
    float lookat[3];
    float vup[3];
    float vfov;
    float aspect;
};

// prepared camera info to make ray creation faster
struct cam_info {
    vec3 origin;
    vec3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
};

struct ispc_hit_info {
    int triangle_index;
    float t;
    float u;
    float v;
};

struct ispc_ray {
    vec3 origin;
    vec3 dir;
};

inline ispc_hit_info new_hit_info() {
    ispc_hit_info hit_info;
    hit_info.triangle_index = -1;
    hit_info.t = 1e30f; // max float
    hit_info.u = 0.f;
    hit_info.v = 0.f;
    return hit_info;
}

inline vec3 a2v(float array[3])
{
    vec3 vec = { array[0], array[1], array[2] };
    return vec;
}

/*
typedef struct tri {
    vec3 vertices[3];
    vec3 normals[3]; // why should a single triangle have 3 normals?
//    vec3f normal;

    vec3 ka;
    vec3 kd;
    vec3 ks;
    vec3 ke;
    float Ns;
    int prim_id; // id of the triangle (face) in the object (geom)
    int geom_id; // id of the object (geom) in the scene
} ispc_triangle;
*/
vec3 new_vec3(float x, float y, float z)
{
    vec3 new_vec3; // create on stack
    new_vec3.x = x;
    new_vec3.y = y;
    new_vec3.z = z;
    return new_vec3;
}

/**
 * Vector dot product
 * C = A.B = AxBx + AyBy + AzBz
 */
float vec3_dot(vec3 a, vec3 b) {
    float dot_product = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    return dot_product;
}

/**
 * Vector cross product
 * formula: C = A x B
 *   Cx = AyBz − AzBy
 *   Cy = AzBx − AxBz
 *   Cz = AxBy − AyBx
 */
inline vec3 vec3_cross(const vec3 a, const vec3 b) {
    vec3 cross_product;
    cross_product.x = (a.y * b.z) - (a.z * b.y);
    cross_product.y = (a.z * b.x) - (a.x * b.z);
    cross_product.z = (a.x * b.y) - (a.y * b.x);
    return cross_product;
}

/**
 * Vector subtraction
 * formula: C = A - B
 *   Cx = Ax - Bx
 *   Cy = Ay - By
 *   Cz = Az - Bz
 */
inline vec3 vec3_subtract(vec3 a, vec3 b) {
    vec3 diff;
    diff.x = a.x - b.x;
    diff.y = a.y - b.y;
    diff.z = a.z - b.z;
    return diff;
}

/**
 * Vector multiply by scalar float
 * formula: C = nA
 *   Cx = Ax * n
 *   Cy = Ay * n
 *   Cz = Az * n
 */
inline vec3 vec3_multiply_scalar(vec3 a,  float n) {
    vec3 product;
    product.x = a.x * n;
    product.y = a.y * n;
    product.z = a.z * n;
    return product;
}

/**
 * Vector divide by scalar float
 * formula: C = A / n
 *   Cx = Ax / n
 *   Cy = Ay / n
 *   Cz = Az / n
 */
inline vec3 vec3_divide_scalar(vec3 a,  float n) {
    vec3 quotient;
    quotient.x = a.x / n;
    quotient.y = a.y / n;
    quotient.z = a.z / n;
    return quotient;
}

/**
 * Vector magnitude
 * formula ||A|| = sqrt(A . A)
 */
inline float vec3_magnitude(const vec3 a)
{
    float magnitude = sqrt(vec3_dot(a, a));
    return magnitude;
}

/**
 * Vector normalize (convert to unit vec3tor in same direction)
 * formula N = A / ||A||
 */
inline vec3 vec3_normalize(const vec3 a) {
    float magnitude = vec3_magnitude(a);
    // vector div scalar using ispc short vectors https://ispc.github.io/ispc.html#short-vector-types
    vec3 normalized = a / magnitude;
    return normalized;
}

inline vec3 normal_to_triangle(ispc_triangle &triangle)
{
    // equivalent of original from main.cpp
    //    auto face = SceneMesh.geometry[i].face_index[f];  // INDEX of hit triangle
    //    auto N = normalize(                               // normalize:
    //       cross(SceneMesh.geometry[i].vertex[face[1]] -  // vert1 - vert0   X  vert2 - vert0
    //             SceneMesh.geometry[i].vertex[face[0]],   // => orthogonal to triangle
    //             SceneMesh.geometry[i].vertex[face[2]] -  // => normalized = normal to triangle
    //             SceneMesh.geometry[i].vertex[face[0]])); //
    // create float<3> ISPC "short vectors" for the 3 corners of the triangle
    vec3 vert0 = { triangle.vertices[0][0], triangle.vertices[0][1], triangle.vertices[0][2] };
    vec3 vert1 = { triangle.vertices[1][0], triangle.vertices[1][1], triangle.vertices[1][2] };
    vec3 vert2 = { triangle.vertices[2][0], triangle.vertices[2][1], triangle.vertices[2][2] };
    // ISPC short vectors support vector subtraction and addition
    vec3 edge1 = vert1 - vert0;
    vec3 edge2 = vert2 - vert0;
    vec3 N = vec3_normalize(vec3_cross(edge1, edge2));
    return N;
}

/**
 * Prepare the camera info to make ray generation faster
 * @param ispc_cam cam the basic camera details
 * @param cam_info info the return object holding pre-calculated uniform info
 */
void prep_cam_info(ispc_cam &cam, cam_info &info) {
    // Originally from the initializer of the camera class in the cpp camera.h
    // vfov is top to bottom in degrees
    float theta = cam.vfov * M_PI / 180;
    float half_height = tan(theta / 2);
    float half_width = cam.aspect * half_height;
    // convert array to ISPC short vector
    vec3 lookfrom_v = { cam.lookfrom[0], cam.lookfrom[1], cam.lookfrom[2] };
    vec3 lookat_v = { cam.lookat[0], cam.lookat[1], cam.lookat[2] };
    vec3 vup_v = { cam.vup[0], cam.vup[1], cam.vup[2] };
    // original cpp code from camera.h
    // w = normalize(lookfrom - lookat);
    // u = normalize(cross(vup, w));
    // v = cross(w, u);
    // lower_left_corner = origin - u * half_width - v * half_height - w;
    // horizontal = u * 2.f * half_width;
    // vertical = v * 2.f * half_height;

    // addition and multiplication-by-scalar using ispc short vectors
    // see here https://ispc.github.io/ispc.html#short-vector-types
    vec3 w = vec3_normalize(lookfrom_v - lookat_v);
    vec3 u = vec3_normalize(vec3_cross(vup_v, w));
    vec3 v = vec3_cross(w, u);

    info.lower_left_corner = info.origin - (u * half_width) - (v * half_height) - w;
    info.horizontal = u *  2.f * half_width;
    info.vertical = v * 2.f * half_height;
    info.origin = lookfrom_v;
}


/**
 * Generate a ray from the view coordinates and camera info
 */
void new_ray(cam_info &info, float s, float t, ispc_ray &ray) {
    // Original cpp code from camera.h:
    //    return ray(origin, normalize(lower_left_corner + horizontal * s + vertical * t - origin));
    ray.origin = info.origin;
    // addition and multiplication-by-scalar using ispc short vectors
    // see here https://ispc.github.io/ispc.html#short-vector-types
    vec3 h_times_s = info.horizontal * s;
    vec3 v_times_t = info.vertical * t;
    vec3 sum = info.lower_left_corner + (info.horizontal * s) + (info.vertical * t) - info.origin;
    ray.dir = vec3_normalize(sum);
}
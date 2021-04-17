#include "ispc_helpers.h"

//int intersect_triangle(ispc_ray &ray, uniform ispc_triangle &triangle, ispc_hit_info &hit_info)
// uniform ispc_triangle &triangle, ispc_hit_info &hit_info)
//int intersect_triangle(ispc_ray &ray, ispc_triangle &triangle)
int intersect_triangle(ispc_ray &ray, ispc_triangle *triangle, ispc_hit_info &hit_info)
{

    vec3 orig = ray.origin;
    vec3 dir = ray.dir;
    vec3 tvec, pvec, qvec;
    double determinant, inverse_determinant;

    // orgiinal from main.cpp:
    // find vectors for two edges sharing vert0
    //    edge1 = vert1 - vert0;
    //    edge2 = vert2 - vert0;
    // create float<3> ISPC "short vectors" for the 3 corners of the triangle
    vec3 vert0 = { triangle->vertices[0][0], triangle->vertices[0][1], triangle->vertices[0][2] };
    vec3 vert1 = { triangle->vertices[1][0], triangle->vertices[1][1], triangle->vertices[1][2] };
    vec3 vert2 = { triangle->vertices[2][0], triangle->vertices[2][1], triangle->vertices[2][2] };
    // ISPC float<3> overrides +- for proper vector addition/subtraction
    vec3 edge1 = vert1 - vert0;
    vec3 edge2 = vert2 - vert0;

    // begin calculating determinant - also used to calculate U parameter
    // pvec is normal to the ray direction and one edge
    pvec = vec3_cross(dir, edge2);
    determinant = vec3_dot(edge1, pvec);

    // if determinant is near zero, ray lies in plane of triangle
    if (determinant > -DBL_EPSILON && determinant < DBL_EPSILON) {
        return 0; // miss
    }

    inverse_determinant = 1.0f / determinant;

    // calculate distance from vert0 to ray origin
    // original from main.cpp:
    //   tvec = orig - vert0;
    tvec = vec3_subtract(orig, vert0);

    //  calculate U parameter and test bounds
    double u2 = vec3_dot(tvec, pvec) * inverse_determinant;
    if (u2 < DBL_EPSILON || u2 > 1.0f) {
        return 0; // miss
    }

    // prepare to test V parameter
    qvec = vec3_cross(tvec, edge1);

    // calculate V parameter and test bounds
    double v2 = vec3_dot(dir, qvec) * inverse_determinant;
    if (v2 < DBL_EPSILON  || u2 + v2 > 1.0f) {
        return 0; // miss
    }

    // calculate t, ray intersects triangle
    double t2 = vec3_dot(edge2, qvec) * inverse_determinant;
    if (t2 < DBL_EPSILON) {
        return 0; // miss
    }

    if (t2 >= hit_info.t) {
        // previous value of t was before this one so we have intersected
        // closer triangle than this one - ignore this triangle
        return 0; // miss
    }
    hit_info.t = t2;
    hit_info.u = u2;
    hit_info.v = v2;

    return 1; // HIT!
}

int intersect_triangles(uniform int num_triangles, uniform ispc_triangle triangles[], // triangles to try to intersect
                        ispc_ray &ray, ispc_hit_info &hit_info, uniform int debug)
{
    // TODO tile loop over CPU vector width
    int has_hit = 0;
    for (int i = 0; i < num_triangles; i++) {
//    foreach (i = 0 ... num_triangles) {
//        has_hit = intersect_triangle(ray, triangles[i]);
        has_hit = intersect_triangle(ray, &triangles[i], hit_info);
        if (has_hit) {
            // TODO find min t here using ispc reduce min
            hit_info.triangle_index = i;
        }
    }
    return has_hit;
}

// TODO make the triangles const and pass in an array
export void trace(uniform int image_width, uniform int image_height,
                  uniform ispc_cam &cam,
                  uniform int num_triangles, uniform ispc_triangle triangles[], // triangles to try to intersect
                  uniform int return_hit_prim_ids[], uniform float return_ray_intersect_ts[], // return values
                  uniform int debug)
{
    if (debug) {
        print("ISPC Trace with w=% h=% targets=% triangles\n", image_width, image_height, num_triangles);
    }

    varying cam_info camera_info;
    varying ispc_cam vcam = cam;
    // precalculate camera info to make ray creation faster in the loop
    prep_cam_info(vcam, camera_info);

    // loop over scene.
    // TODO consider foreach_tiled see https://ispc.github.io/ispc.html#parallel-iteration-statements-foreach-and-foreach-tilede
    foreach (h = 0 ... image_height, w = 0 ... image_width) {
        int geom_id = -1;
        int prim_id = 0;

        float is = ((float)w) / (image_width - 1);
        float it = ((float)h) / (image_height - 1);
        ispc_ray ray;
        new_ray(camera_info, is, it, ray);
        ispc_hit_info hit_info = new_hit_info();

        if (intersect_triangles(num_triangles, triangles, ray, hit_info, debug)) {
            // original code:
            //    auto i = geomID;
            //    auto f = primID;    // f = triangle ID, face = array of triangle corner indexes in geom.vertex
            //    auto face = SceneMesh.geometry[i].face_index[f];  // INDEX of hit triangle
            ispc_triangle tri = triangles[hit_info.triangle_index];
            vec3 N; // normal to hit triangle
            if (!tri.has_normals) {
                N = normal_to_triangle(tri);
            }
            else {
                // orginal cpp code:
                //   if (!SceneMesh.geometry[i].normals.empty()) {
                //     auto N0 = SceneMesh.geometry[i].normals[face[0]];
                //     auto N1 = SceneMesh.geometry[i].normals[face[1]];
                //     auto N2 = SceneMesh.geometry[i].normals[face[2]];
                //     N = normalize(N1 * u + N2 * v + N0 * (1 - u - v));
                vec3 N0 = {tri.normals[0][0], tri.normals[0][1], tri.normals[0][2] };
                vec3 N1 = {tri.normals[1][0], tri.normals[1][1], tri.normals[1][2] };
                vec3 N2 = {tri.normals[2][0], tri.normals[2][1], tri.normals[2][2] };
                N = vec3_normalize((N1 * hit_info.u) + (N2 * hit_info.v) + (N0 * (1 - hit_info.u - hit_info.v)));
            }
       }
   }
}

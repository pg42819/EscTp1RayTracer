#include "ispc_helpers.h"

//int intersect_triangle(ispc_ray &ray, uniform ispc_triangle &triangle, ispc_hit_info &hit_info)
// uniform ispc_triangle &triangle, ispc_hit_info &hit_info)
//int intersect_triangle(ispc_ray &ray, ispc_triangle &triangle)
int intersect_triangle(ispc_ray &ray, ispc_triangle *triangle, int triangle_index, ispc_hit_info &hit_info)
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
    vec3 vert0 = {triangle->vertices[0][0], triangle->vertices[0][1], triangle->vertices[0][2]};
    vec3 vert1 = {triangle->vertices[1][0], triangle->vertices[1][1], triangle->vertices[1][2]};
    vec3 vert2 = {triangle->vertices[2][0], triangle->vertices[2][1], triangle->vertices[2][2]};
    // ISPC float<3> overrides +- for proper vector addition/subtraction
    vec3 edge1 = vert1 - vert0;
    vec3 edge2 = vert2 - vert0;

    // begin calculating determinant - also used to calculate U parameter
    // pvec is normal to the ray direction and one edge
    pvec = vec3_cross(dir, edge2);
    determinant = vec3_dot(edge1, pvec);

    // if determinant is near zero, ray lies in plane of triangle
    if (determinant <= -EPSILON || determinant >= EPSILON) {
        inverse_determinant = 1.0f / determinant;

        // calculate distance from vert0 to ray origin
        // original from main.cpp:
        //   tvec = orig - vert0;
        tvec = vec3_subtract(orig, vert0);

        //  calculate U parameter and test bounds
        double u2 = vec3_dot(tvec, pvec) * inverse_determinant;
        if (u2 >= EPSILON && u2 <= 1.0f) {

            // prepare to test V parameter
            qvec = vec3_cross(tvec, edge1);

            // calculate V parameter and test bounds
            double v2 = vec3_dot(dir, qvec) * inverse_determinant;
            if (v2 >= EPSILON && u2 + v2 <= 1.0f) {

                // calculate t, ray intersects triangle
                double t2 = vec3_dot(edge2, qvec) * inverse_determinant;

                // reverse the logic
                if (t2 > EPSILON) {
                    // reversing original logic for easier use across instances
                    if (t2 < hit_info.t) {
                        // previous value of t was further than this one so we have a new winner
                        hit_info.t = t2;
                        hit_info.u = u2;
                        hit_info.v = v2;

                        hit_info.triangle_index = triangle_index; // HIT!
                    }
                }
            }
        }
    }
}

// TODO implement occlusion
int occlusion(vec3 hit, vec3 L, float t)
{
    return 0;
}

void intersect_triangles(uniform int num_triangles, uniform ispc_triangle triangles[], // triangles to try to intersect
                         ispc_ray &ray, ispc_hit_info &hit_info, uniform int debug)
{
    // TODO tile loop over CPU vector width
    if (debug >= TRACE) {
        print("Checking triangles incoming \n    i=%\n    hit_info:\n        tri=%\n        t=%\n        u=%\n\n",
              hit_info.triangle_index, hit_info.t, hit_info.u);
    }
//    int has_hit = 0;
    foreach (i = 0 ... num_triangles) {
        intersect_triangle(ray, &triangles[i], i, hit_info);
    }
    if (debug >= TRACE) {
        print("Checked triangles outgoing \n    i=%\n    hit_info:\n         tri=%\n        t=%\n        u=%\n\n",
              hit_info.triangle_index, hit_info.t, hit_info.u);
    }
//    if (any(hit_info.triangle_index >= 0)) {
//        has_hit = 1;
//    }
//    return has_hit;
}

export void trace(uniform int image_width, uniform int image_height,
                  uniform ispc_cam &cam,
                  uniform int num_triangles, uniform ispc_triangle triangles[], // triangles to try to intersect
                  uniform int num_lights, uniform ispc_light lights[],
                  uniform int num_light_triangles, uniform ispc_triangle light_triangles[],
                  uniform float * uniform return_image,
                  uniform int debug, uniform int test)
{
    if (debug >= DEBUG) {
        print("ISPC Trace with w=% h=% targets=% triangles\n", image_width, image_height, num_triangles);
        print("     num_lights=%  num_light_triangles=%\n", num_lights, num_light_triangles);
        uniform int last = num_lights - 1;
        print("     Last light: geom_id=%  num_light_faces=% last_face_index=%\n",
              lights[last].geom_id, lights[last].num_light_faces, lights[last].light_faces[lights[last].num_light_faces - 1]);
    }

// ISPC Random Number Generator: see https://ispc.github.io/ispc.html#pseudo-random-numbers
    // Using the program index to vary the seed across program instances
    // Documentation doesn't say what the range of random and frandom are but I found it here:
    // https://groups.google.com/g/ispc-users/c/ka4G_FP4j2A/m/PVzay9NyZ4wJ
    // frandom is [0.0, 1.0) and random is all unsigned ints
    RNGState rngstate;
    seed_rng(&rngstate, programIndex);

    varying cam_info camera_info;
    varying ispc_cam vcam = cam;
    // precalculate camera info to make ray creation faster in the loop
    prep_cam_info(vcam, camera_info);

    // loop over scene.
    // TODO consider foreach_tiled see https://ispc.github.io/ispc.html#parallel-iteration-statements-foreach-and-foreach-tilede
    //    foreach (h = 0 ... image_height, w = 0 ... image_width)
    for (int h = image_height - 1; h >= 0; --h) {
        for (int w = 0; w < image_width; ++w) {
            float is = ((float)w) / (image_width - 1);
            float it = ((float)h) / (image_height - 1);
            ispc_ray ray;
            new_ray(camera_info, is, it, ray);
            ispc_hit_info hit_info = new_hit_info();

            intersect_triangles(num_triangles, triangles, ray, hit_info, debug);
            if (hit_info.triangle_index >= 0) {
                // original code:
                //    auto i = geomID;
                //    auto f = primID;    // f = triangle ID, face = array of triangle corner indexes in geom.vertex
                //    auto face = SceneMesh.geometry[i].face_index[f];  // INDEX of hit triangle
                ispc_triangle tri = triangles[hit_info.triangle_index];

                // used later for lighting:
                material mat;
                get_material(tri, mat);
                if (debug >= DEBUG) {
                    print("Hit triangle #%\n  at t=% \n", hit_info.triangle_index, hit_info.t);
                }

                vec3 N; // normal to hit triangle
                // Unlike the original version we only create the normals here if
                // we are not going to use the existing ones (the original did it and overwrote)
                if (!tri.has_normals) {
                    N = normal_to_triangle(tri);
                }
                else {
                    // orginal cpp code:
                    //   if (!SceneMesh.geometry[i].normals.empty())
                    //     auto N0 = SceneMesh.geometry[i].normals[face[0]];
                    //     auto N1 = SceneMesh.geometry[i].normals[face[1]];
                    //     auto N2 = SceneMesh.geometry[i].normals[face[2]];
                    //     N = normalize(N1 * u + N2 * v + N0 * (1 - u - v));
                    vec3 N0 = { tri.normals[0][0], tri.normals[0][1], tri.normals[0][2] };
                    vec3 N1 = { tri.normals[1][0], tri.normals[1][1], tri.normals[1][2] };
                    vec3 N2 = { tri.normals[2][0], tri.normals[2][1], tri.normals[2][2] };
                    N = vec3_normalize((N1 * hit_info.u) + (N2 * hit_info.v) + (N0 * (1 - hit_info.u - hit_info.v)));
                }

                // now we have the hit triangle and its normal use the lightsources to get the color
                // Original from main.cpp:
                //  for (auto &lightID : SceneMesh.light_sources)
                //     light = SceneMesh.geometry[lightID];
                //     light.face_index.size();
                //     std::uniform_int_distribution<int> distrib1(0, light.face_index.size() - 1);
                //     int faceID = distrib1(gen);
                for (uniform int light_index = 0; light_index < num_lights; light_index++) {
                    if (debug >= TRACE) {
                        print("looping in light_index % of %", light_index, num_lights);
                    }

                    // light is an object (geom) with a set of ids of the faces in the same object that are lights
                    int random_light_face_index = random(&rngstate) % lights[light_index].num_light_faces;

                    // get the index into the triangles list of the light face selected at random
                    // from the light faces owned by this light object
                    int light_face_index = lights[light_index].light_faces[random_light_face_index];
                    // find the triangle in the list whose index matches the chosen face from the array of indices
                    ispc_triangle light_face = light_triangles[light_face_index];

                    if (debug >= TRACE) {
                        print("Selecting random face index: % in light source: %\n", random_light_face_index, light_index);
                        print("   light_face_index in light triangles = %   light_triangle.geom = %\n",
                              light_face_index, light_face.geom_id);
                    }

// Original from main.cpp: Note is this a bug? All 3 v0,v1,v2 are the same vertex
                    // so v1 - v0 and v2 - v0 are zero!?
                    //    const auto &v0 = light.vertex[faceID];
                    //    const auto &v1 = light.vertex[faceID];
                    //    const auto &v2 = light.vertex[faceID];
                    // NOTE different from the original algo: we are using different vertices for the 3 vs!
                    vec3 v0 = { light_face.vertices[0][0], light_face.vertices[0][1], light_face.vertices[0][2] };
                    vec3 v1 = { light_face.vertices[1][0], light_face.vertices[1][1], light_face.vertices[1][2] };
                    vec3 v2 = { light_face.vertices[2][0], light_face.vertices[2][1], light_face.vertices[2][2] };

                    // Original from main.cpp:
                    //    auto P = v0 + ((v1 - v0) * float(distrib(gen)) +
                    //                   (v2 - v0) * float(distrib(gen)));
                    vec3 P = v0 + ((v1 - v0) * frandom(&rngstate) +
                            (v2 - v0) * frandom(&rngstate));

                    // Original from main.cpp
                    //    auto hit = ray.origin +
                    //           ray.dir * (t - std::numeric_limits<float>::epsilon());
                    //    auto L = P - hit;
                    //    auto len = tracer::length(L);
                    //    t = len - std::numeric_limits<float>::epsilon();
                    //    L = tracer::normalize(L);
                    // point of hit = origin point following ray direction for time-of-flight (backing off slightly)
                    vec3 hit = ray.origin + ray.dir * (hit_info.t - EPSILON);
                    vec3 L = P - hit;
                    float len = vec3_magnitude(L);
                    float t = len - EPSILON;
                    L = vec3_normalize(L);

                    //
                    // Original from main.cpp
                    // auto mat = SceneMesh.geometry[i].object_material;
                    // auto c =
                    //        (mat.ka * 0.5f + mat.ke) / float(SceneMesh.light_sources.size());
                    // c is the color vector (r,g,b)
                    // divide color by number of lights to get the fraction of the effect of this light source
                    vec3 c = (mat.ka * 0.5f + mat.ke) / ((float)num_lights);

                    // TODO __ occlusion
                    // Original from main.cpp
                    //     if (occlusion(SceneMesh, hit, L, t))
                    //         continue;
                    if (occlusion(hit, L, t)) {
                        continue;
                    }

                    //     auto d = dot(N, L);
                    //    if (d <= 0)
                    //       continue;
                    //
                    //     auto H = normalize((N + L) * 2.f);
                    //
                    //     c = c + (mat.kd * d + mat.ks * pow(dot(N, H), mat.Ns)) /
                    //        float(SceneMesh.light_sources.size());
                    float d = vec3_dot(N, L);
                    if (d <= 0) {
                        continue;
                    }

                    vec3 H = vec3_normalize((N + L) * 2.f);
                    c = c + (mat.kd * d + mat.ks * pow(vec3_dot(N, H), mat.Ns)) / ((float)num_lights);

                    // Original from main.cpp
                    //     image[h * image_width + w].r += c.r;
                    //     image[h * image_width + w].g += c.g;
                    //     image[h * image_width + w].b += c.b;

                    // add colors to build up final color of pixel over light sources
                    // in flat ispc image, r, g, b are inline in the array for each pixel, hence * 3;
                    int offset = (h * image_width + w) * 3;
                    return_image[offset + 0] += c.r;
                    return_image[offset + 1] += c.g;
                    return_image[offset + 2] += c.b;
                } // foreach light source
            } // if ray hits triangle
        }// for each scene pixel w
    }// for each scene pixel h
}


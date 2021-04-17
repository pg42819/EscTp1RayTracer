#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <float.h>
#include "math/vec.h"
#include "scene/camera.h"
#include "scene/ray_triangle.h"
#include "scene/scene.h"
#include "scene/sceneloader.h"
#include "simplify/c_triangle.h"
#include "simplify/flatten.h"
#include "scene/aabb.h"
#include "scene/bvh.h"
#include "simplify/flatten_iscp.h"
// headers generated by ispc compiler
//#include "test_ispc.h"
#include "trace_ispc.h"

void
scan_row(tracer::scene &SceneMesh, int image_width, int image_height, tracer::camera &cam, tracer::vec3<float> *image,
         std::mt19937 &gen, std::uniform_real_distribution<float> &distrib, int h);


c_vec3f vec3ToCVec3(tracer::vec3<float> input) {
    c_vec3f r;

    r.x = input.x;
    r.y = input.y;
    r.z = input.z;

    return r;
}

aabb computeMaxBox(const c_triangle *c_triangles, int begin, int end) {
    c_vec3f minVertex, maxVertex;

    minVertex = new_vec3f(INT_MAX,INT_MAX,INT_MAX);
    maxVertex = new_vec3f(INT_MIN,INT_MIN,INT_MIN);

    if(begin == end) {
        int i;

        for(i = 0; i < 3; i++) {
            if(c_triangles[begin].vertices[i].x > maxVertex.x)
                maxVertex.x = c_triangles[begin].vertices[i].x;
            if(c_triangles[begin].vertices[i].y > maxVertex.y)
                maxVertex.y = c_triangles[begin].vertices[i].y;
            if(c_triangles[begin].vertices[i].z > maxVertex.z)
                maxVertex.z = c_triangles[begin].vertices[i].z;

            if(c_triangles[begin].vertices[i].x < minVertex.x)
                minVertex.x = c_triangles[begin].vertices[i].x;
            if(c_triangles[begin].vertices[i].y < minVertex.y)
                minVertex.y = c_triangles[begin].vertices[i].y;
            if(c_triangles[begin].vertices[i].z < minVertex.z)
                minVertex.z = c_triangles[begin].vertices[i].z;
        }

    }
    else {
        int w, j;

        for(w = begin; w <= end; w++) {
            for(j = 0; j < 3; j++) {
                if(c_triangles[w].vertices[j].x > maxVertex.x)
                maxVertex.x = c_triangles[w].vertices[j].x;
                if(c_triangles[w].vertices[j].y > maxVertex.y)
                    maxVertex.y = c_triangles[w].vertices[j].y;
                if(c_triangles[w].vertices[j].z > maxVertex.z)
                    maxVertex.z = c_triangles[w].vertices[j].z;

                if(c_triangles[w].vertices[j].x < minVertex.x)
                    minVertex.x = c_triangles[w].vertices[j].x;
                if(c_triangles[w].vertices[j].y > minVertex.y)
                    minVertex.y = c_triangles[w].vertices[j].y;
                if(c_triangles[w].vertices[j].z > minVertex.z)
                    minVertex.z = c_triangles[w].vertices[j].z;
            }
        }

    }

    return aabb(minVertex, maxVertex);

}

bvh_node * buildBVH(bvh_node *tree, const tracer::scene &SceneMesh, int begin, int end, bool threaded) {

    int middle = 0;
    middle += begin + (end-begin) / 2;

    std::vector<std::thread> threads;

    if(threaded) {
        if(begin == end) {

        aabb box = computeMaxBox(SceneMesh.c_triangles, begin, begin);

        tree->bbox = box;

        tree->primitive_count = end - begin + 1;

        tree->start = begin;

        tree->left = NULL;
        tree->right = NULL;

        } else {
            aabb box = computeMaxBox(SceneMesh.c_triangles, begin, end);

            tree->bbox = box;

            tree->primitive_count = end - begin + 1;

            tree->start = begin;

            tree->left = (bvh_node *)malloc(sizeof(struct bvh_node));
            tree->right = (bvh_node *)malloc(sizeof(struct bvh_node));
            threads.push_back(std::thread(buildBVH,std::ref(tree->left), std::ref(SceneMesh), begin, middle, threaded));
            threads.push_back(std::thread(buildBVH,std::ref(tree->right), std::ref(SceneMesh), middle+1, end, threaded));
            //tree->right = buildBVH(tree->right, SceneMesh, middle+1, end, threaded);
        }
    } else {
        if(begin == end) {

        aabb box = computeMaxBox(SceneMesh.c_triangles, begin, begin);

        tree->bbox = box;

        tree->primitive_count = end - begin + 1;

        tree->start = begin;

        tree->left = NULL;
        tree->right = NULL;

        } else {
            aabb box = computeMaxBox(SceneMesh.c_triangles, begin, end);

            tree->bbox = box;

            tree->primitive_count = end - begin + 1;

            tree->start = begin;

            tree->left = (bvh_node *)malloc(sizeof(struct bvh_node));
            tree->right = (bvh_node *)malloc(sizeof(struct bvh_node));
            tree->left = buildBVH(tree->left, SceneMesh, begin, middle, threaded);
            tree->right = buildBVH(tree->right, SceneMesh, middle+1, end, threaded);
        }
    }

    for (auto &thr : threads) {
            thr.join();
        }

    return tree;
}

// Loops through every Face on every object (Geometry) and checks if a ray at the
// given original and direction will intersect it.
// Returns true and sets the ID of the object in geomID and face in primID
bool cpp_intersect(const tracer::scene &SceneMesh, const tracer::vec3<float> &ori,
                   const tracer::vec3<float> &dir, float &t, float &u, float &v,
                   size_t &geomID, size_t &primID) {
    for (auto i = 0; i < SceneMesh.geometry.size(); i++) {
    for (auto f = 0; f < SceneMesh.geometry[i].face_index.size(); f++) {
      auto face = SceneMesh.geometry[i].face_index[f];
      if (tracer::intersect_triangle(
              ori, dir, SceneMesh.geometry[i].vertex[face[0]],
              SceneMesh.geometry[i].vertex[face[1]],
              SceneMesh.geometry[i].vertex[face[2]], t, u, v)) {
        geomID = i;
        primID = f;
      }
    }
  }
  return (geomID != -1 && primID != -1);
}

void c_intersect_triangles_limits(c_triangle *triangles, int num_triangles,
                           c_vec3f origin, c_vec3f direction,
                           float *near_t_ptr, float *u_ptr, float *v_ptr,
                           int *geom_id, int *prim_id, int begin, int end)
{
    for (auto i = begin; i < (begin+end); i++) {
        c_triangle triangle = triangles[i];
        // note that near_t_ptr keeps track of the shortest time-of-flight to hit a triangle:
        // intersect will only return true if it hits a triangle at t < *near_t_ptr
        if (c_intersect_triangle(origin, direction, &triangle, near_t_ptr, u_ptr, v_ptr)) {
            *geom_id = triangle.geom_id;
            *prim_id = triangle.prim_id;
            // keep looping even if we hit a triangle in case we hit another one
            // that is closer to the ray origin (ie with a smaller value of  later value of t)
        }
    }
}

void c_intersect_triangles(c_triangle *triangles, int num_triangles,
                           c_vec3f origin, c_vec3f direction,
                           float *near_t_ptr, float *u_ptr, float *v_ptr,
                           int *geom_id, int *prim_id)
{
    for (auto i = 0; i < num_triangles; i++) {
        c_triangle triangle = triangles[i];
        // note that near_t_ptr keeps track of the shortest time-of-flight to hit a triangle:
        // intersect will only return true if it hits a triangle at t < *near_t_ptr
        if (c_intersect_triangle(origin, direction, &triangle, near_t_ptr, u_ptr, v_ptr)) {
            *geom_id = triangle.geom_id;
            *prim_id = triangle.prim_id;
            // keep looping even if we hit a triangle in case we hit another one
            // that is closer to the ray origin (ie with a smaller value of  later value of t)
        }
    }
}

// Loops through every Face on every object (Geometry) and checks if a ray at the
// given original and direction will intersect it.
// Returns true and sets the ID of the object in geomID and face in primID
bool call_c_intersect_limits(const tracer::scene &SceneMesh, const tracer::vec3<float> &ori,
                      const tracer::vec3<float> &dir, float &t, float &u, float &v,
                      size_t &geomID, size_t &primID, int begin, int end)
{
    c_vec3f c_origin = cpp_vec_to_c_vec(ori);
    c_vec3f c_direction = cpp_vec_to_c_vec(dir);
    // create new t, u, v for returning values from callee
    float c_t = t;
    float c_u = u;
    float c_v = v;
    int geom_id = -1;
    int prim_id = -1;
    c_intersect_triangles_limits(SceneMesh.c_triangles, SceneMesh.num_triangles, c_origin, c_direction,
                          &c_t, &c_u, &c_v, &geom_id, &prim_id, begin, end);
    t = c_t;
    u = c_u;
    v = c_v;
    geomID = geom_id;
    primID = prim_id;

    // return true if we hit a triangle
    return (geomID != -1 && primID != -1);
}

// Loops through every Face on every object (Geometry) and checks if a ray at the
// given original and direction will intersect it.
// Returns true and sets the ID of the object in geomID and face in primID
bool call_c_intersect(const tracer::scene &SceneMesh, const tracer::vec3<float> &ori,
                      const tracer::vec3<float> &dir, float &t, float &u, float &v,
                      size_t &geomID, size_t &primID)
{
    c_vec3f c_origin = cpp_vec_to_c_vec(ori);
    c_vec3f c_direction = cpp_vec_to_c_vec(dir);
    // create new t, u, v for returning values from callee
    float c_t = t;
    float c_u = u;
    float c_v = v;
    int geom_id = -1;
    int prim_id = -1;
    c_intersect_triangles(SceneMesh.c_triangles, SceneMesh.num_triangles, c_origin, c_direction,
                          &c_t, &c_u, &c_v, &geom_id, &prim_id);
    t = c_t;
    u = c_u;
    v = c_v;
    geomID = geom_id;
    primID = prim_id;

    // return true if we hit a triangle
    return (geomID != -1 && primID != -1);
}

// Loops through every Face on every object (Geometry) and checks if a ray at the
// given original and direction will intersect it.
// Returns true and sets the ID of the object in geomID and face in primID
bool intersect_limits(const tracer::scene &SceneMesh, const tracer::vec3<float> &ori,
               const tracer::vec3<float> &dir, float &t, float &u, float &v,
               size_t &geomID, size_t &primID, int begin, int end) {
    if (SceneMesh.num_triangles > 0) {
        // scene has flattened triangles, use the c_intersect method
        return call_c_intersect_limits(SceneMesh, ori, dir, t, v, v, geomID, primID, begin, end);
    }
    else {
        return cpp_intersect(SceneMesh, ori, dir, t, v, v, geomID, primID);
    }
}

// Loops through every Face on every object (Geometry) and checks if a ray at the
// given original and direction will intersect it.
// Returns true and sets the ID of the object in geomID and face in primID
bool intersect(const tracer::scene &SceneMesh, const tracer::vec3<float> &ori,
               const tracer::vec3<float> &dir, float &t, float &u, float &v,
               size_t &geomID, size_t &primID) {
    if (SceneMesh.num_triangles > 0) {
        // scene has flattened triangles, use the c_intersect method
        return call_c_intersect(SceneMesh, ori, dir, t, v, v, geomID, primID);
    }
    else {
        return cpp_intersect(SceneMesh, ori, dir, t, v, v, geomID, primID);
    }
}

bool occlusion(const tracer::scene &SceneMesh, const tracer::vec3<float> &ori,
               const tracer::vec3<float> &dir, float &t) {
  float u, v;
  for (auto i = 0; i < SceneMesh.geometry.size(); i++) {
    for (auto f = 0; f < SceneMesh.geometry[i].face_index.size(); f++) {
      auto face = SceneMesh.geometry[i].face_index[f];
      if (tracer::intersect_triangle(
              ori, dir, SceneMesh.geometry[i].vertex[face[0]],
              SceneMesh.geometry[i].vertex[face[1]],
              SceneMesh.geometry[i].vertex[face[2]], t, u, v)) {
        return true;
      }
    }
  }
  return false;
}

int main(int argc, char *argv[]) {
    std::string modelname;
    std::string outputname;
    bool hasEye{false}, hasLook{false};
    bool threaded{false};
    bool flat{false};
    bool ispc{false};
    int debug{0};
    tracer::vec3<float> eye(0, 1, 3), look(0, 1, 0);
    tracer::vec2<uint> windowSize(1024, 768);

    // command line arguments
    for (int arg = 0; arg < argc; arg++) {

        // --thread selects the threaded model
        if (std::string(argv[arg]) == "--thread") {
            threaded = true;
            continue;
        }

        // --flat flattens the scenemesh into triangles
        if (std::string(argv[arg]) == "--flat") {
            flat = true;
            continue;
        }

        // --ispc invokes the ispc parallel implementation
        if (std::string(argv[arg]) == "--ispc") {
            ispc = true;
            flat = true;
            continue;
        }

        // --debug
        if (std::string(argv[arg]) == "--debug") {
            debug = 1;
            continue;
        }

        // -m model.obj : path to the model file
        if (std::string(argv[arg]) == "-m") {
            modelname = std::string(argv[arg + 1]);
            arg++;
            continue;
        }

        // -o output.ppm : path to output the rendered image
        if (std::string(argv[arg]) == "-o") {
            outputname = std::string(argv[arg + 1]);
            arg++;
            continue;
        }

        // -v x,y,z : position of the eye, defaults to 0, 1, 3
        if (std::string(argv[arg]) == "-v") {
            char *token = std::strtok(argv[arg + 1], ",");
            int i = 0;
            while (token != NULL) {
                eye[i++] = atof(token);
                token = std::strtok(NULL, ",");
            }

            if (i != 3)
                throw std::runtime_error("Error parsing view");
            hasEye = true;
            arg++;
            continue;
        }

        // -l x,y,z : direction to look in, defaults to 0,1,0
        if (std::string(argv[arg]) == "-l") {
            char *token = std::strtok(argv[arg + 1], ",");
            int i = 0;

            while (token != NULL) {
                look[i++] = atof(token);
                token = std::strtok(NULL, ",");
            }

            if (i != 3)
                throw std::runtime_error("Error parsing look");
            hasLook = true;
            arg++;
            continue;
        }

        // -w w,h : size of the window, defaults to
        if (std::string(argv[arg]) == "-w") {
            char *token = std::strtok(argv[arg + 1], ",");
            int i = 0;

            while (token != NULL) {
                look[i++] = atof(token);
                token = std::strtok(NULL, ",");
            }

            if (i != 2)
                throw std::runtime_error("Error parsing window size");
            hasLook = true;
            arg++;
            continue;
        }

        if (arg > 0) { // arg==0 is the program so ignore it - anything else that gets here is unexpected
            std::string error = std::string("Invalid Argument: ") + argv[arg];
            throw std::runtime_error(error);
        }
    }

    tracer::scene SceneMesh;
    bool ModelLoaded = false;

    if (modelname != "") {
        SceneMesh = model::loadobj(modelname);
        ModelLoaded = true;
    }

    int image_width = windowSize.x;
    int image_height = windowSize.y;

    tracer::camera cam(eye, look, tracer::vec3<float>(0, 1, 0), 60,
                       float(image_width) / image_height);
    // Render

    tracer::vec3<float> *image =
            new tracer::vec3<float>[image_height * image_width];

    SceneMesh.tree = (bvh_node *)malloc(sizeof(struct bvh_node));

    // flatten if needed
    if (flat) {
        flatten_scene(SceneMesh);

        SceneMesh.tree = buildBVH(SceneMesh.tree, SceneMesh, 0, SceneMesh.num_triangles-1, threaded);

        bvh_node *t = SceneMesh.tree;

        while(t != NULL) {
            std::cout << "STARTS --> " << t->start << " AND HAS #PRIMITIVES " << t->primitive_count << "\n";

            t = t->right->left;
        }
    }
    ispc::ispc_triangle *flat_triangles;
    int num_flat_triangles = 0;

    // start the clock!
    auto start_time = std::chrono::high_resolution_clock::now();

    std::cout << SceneMesh.geometry.size() << " <-- THIS IS HOW MANY OBJS THERE ARE";

    // START HERE
    std::vector<std::thread> threads;


    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distrib(0, 1.f);


    if (ispc) {
        // note this happens after clock start to compare with algos that do not need to flatten
        // flatten if needed
        flatten_scene_ispc(SceneMesh, &flat_triangles, &num_flat_triangles);
        // convert camera to ispc camera
        ispc::ispc_cam ispc_camera;

        int *hit_triangle_prim_ids = new int[image_width * image_height];
        float *ray_intersect_ts = new float[image_width * image_height];
        ispc::trace(image_width, image_height, ispc_camera, num_flat_triangles, flat_triangles,
                    /* return values*/hit_triangle_prim_ids, ray_intersect_ts,
                    debug);
//        ispc::test_vectors_ispc();
    }
    else {
        // scan vertically and horizontally for each point in the image window...
        for (int h = image_height - 1; h >= 0; --h) {
            if (threaded) {
                // trace rays on a single row in the image window
                threads.push_back(std::thread(scan_row, std::ref(SceneMesh), image_width, image_height,
                                              std::ref(cam), image, std::ref(gen), std::ref(distrib), h));
            } else {
                scan_row(SceneMesh, image_width, image_height, cam, image, gen, distrib, h);
            }
        }
    }

    if (threaded) {
        for (auto &thr : threads) {
            thr.join();
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    std::cerr << "\n Threaded  : " << std::boolalpha << threaded << std::endl;
    std::cerr <<   " Flattened : " << std::boolalpha << flat << std::endl;
    std::cerr <<   " ISPC      : " << std::boolalpha << ispc << std::endl;
    std::cerr << "\n Duration  : "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                                       start_time)
                      .count()
              << std::endl;

    if (!outputname.empty()) {
        std::ofstream file(outputname, std::ios::out);

        file << "P3\n" << image_width << " " << image_height << "\n255\n";
        for (int h = image_height - 1; h >= 0; --h) {
            for (int w = 0; w < image_width; ++w) {
                auto &img = image[h * image_width + w];
                img.r = (img.r > 1.f) ? 1.f : img.r;
                img.g = (img.g > 1.f) ? 1.f : img.g;
                img.b = (img.b > 1.f) ? 1.f : img.b;

                file << int(img.r * 255) << " " << int(img.g * 255) << " "
                     << int(img.b * 255) << "\n";
            }
        }

        std::cout << "Rendered image in: " << outputname << std::endl;
    }
    else {
        std::cout << "Nothing saved: use -o to save rendered image" << std::endl;
    }
    delete[] image;
    return 0;
}

void
scan_row(tracer::scene &SceneMesh, int image_width, int image_height, tracer::camera &cam,
         tracer::vec3<float> *image,
         std::mt19937 &gen, std::uniform_real_distribution<float> &distrib, int h)
{
    for (int w = 0; w < image_width; w++) {
        size_t geomID = -1;
        size_t primID = -1;

        // is = horizontal fraction, it = vertical fraction of the image
        auto is = float(w) / (image_width - 1);
        auto it = float(h) / (image_height - 1);

        // get a ray from the origin pointing to this point on the window
        auto ray = cam.get_ray(is, it);

        float t = std::numeric_limits<float>::max();
        float u = 0;
        float v = 0;

        if(SceneMesh.num_triangles == 0) {

            // if this ray intersects a polygon on the object, get the id of the face and object
            // and the details of intersection (t, u , v)
            if (intersect(SceneMesh, ray.origin, ray.dir, t, u, v, geomID, primID)) {
                auto i = geomID;
                auto f = primID;
                auto face = SceneMesh.geometry[i].face_index[f];

                // find the normal vector for the face
                auto N = normalize(cross(SceneMesh.geometry[i].vertex[face[1]] -
                                        SceneMesh.geometry[i].vertex[face[0]],
                                        SceneMesh.geometry[i].vertex[face[2]] -
                                        SceneMesh.geometry[i].vertex[face[0]]));

                if (!SceneMesh.geometry[i].normals.empty()) {
                    auto N0 = SceneMesh.geometry[i].normals[face[0]];
                    auto N1 = SceneMesh.geometry[i].normals[face[1]];
                    auto N2 = SceneMesh.geometry[i].normals[face[2]];
                    N = normalize(N1 * u + N2 * v + N0 * (1 - u - v));
                }

                for (auto &lightID : SceneMesh.light_sources) {
                    auto light = SceneMesh.geometry[lightID];
                    light.face_index.size();
                    std::uniform_int_distribution<int> distrib1(
                            0, light.face_index.size() - 1);

                    // To draw a random variable from the distribution, you use the function call operator()
                    // and pass in an instance of a random number engine, such as a Mersenne Twister.
                    int faceID = distrib1(gen);
                    const auto &v0 = light.vertex[faceID];
                    const auto &v1 = light.vertex[faceID];
                    const auto &v2 = light.vertex[faceID];

                    auto P = v0 + ((v1 - v0) * float(distrib(gen)) +
                                (v2 - v0) * float(distrib(gen)));

                    // hit is where along the vector it hits the face
                    auto hit = ray.origin +
                            ray.dir * (t - std::numeric_limits<float>::epsilon());
                    auto L = P - hit;

                    auto len = tracer::length(L);

                    // 191 original: check time of flight to see if we have hit occlusion
                    t = len - std::numeric_limits<float>::epsilon();

                    L = tracer::normalize(L);

                    auto mat = SceneMesh.geometry[i].object_material;
                    auto c =
                            (mat.ka * 0.5f + mat.ke) / float(SceneMesh.light_sources.size());

                    if (occlusion(SceneMesh, hit, L, t))
                        continue;

                    auto d = dot(N, L);

                    if (d <= 0)
                        continue;

                    auto H = normalize((N + L) * 2.f);

                    c = c + (mat.kd * d + mat.ks * pow(dot(N, H), mat.Ns)) /
                            float(SceneMesh.light_sources.size());

                    // set pixel on the result image to the calculated color
                    image[h * image_width + w].r += c.r;
                    image[h * image_width + w].g += c.g;
                    image[h * image_width + w].b += c.b;
                }
            }
        } else {

                float tnear, tfar;
                c_vec3f ori = vec3ToCVec3(ray.origin);
                c_vec3f dir = vec3ToCVec3(ray.dir);

                std::cout << "MIN X,Y,Z BBOX --> " << SceneMesh.tree->bbox._min.x << "," << SceneMesh.tree->bbox._min.y << "," << SceneMesh.tree->bbox._min.z << "\n";

                std::cout << "MAX X,Y,Z BBOX --> " << SceneMesh.tree->bbox._max.x << "," << SceneMesh.tree->bbox._max.y << "," << SceneMesh.tree->bbox._max.z << "\n";

                if(SceneMesh.tree->bbox.intersect(ori, dir, &tnear, &tfar)) {

                    if(intersect(SceneMesh, ray.origin, ray.dir, t, u, v, geomID, primID)) {
                    auto i = geomID;
                    auto f = primID;
                    auto face = SceneMesh.geometry[i].face_index[f];

                    // find the normal vector for the face
                    auto N = normalize(cross(SceneMesh.geometry[i].vertex[face[1]] -
                                            SceneMesh.geometry[i].vertex[face[0]],
                                            SceneMesh.geometry[i].vertex[face[2]] -
                                            SceneMesh.geometry[i].vertex[face[0]]));

                    if (!SceneMesh.geometry[i].normals.empty()) {
                        auto N0 = SceneMesh.geometry[i].normals[face[0]];
                        auto N1 = SceneMesh.geometry[i].normals[face[1]];
                        auto N2 = SceneMesh.geometry[i].normals[face[2]];
                        N = normalize(N1 * u + N2 * v + N0 * (1 - u - v));
                    }

                    for (auto &lightID : SceneMesh.light_sources) {
                        auto light = SceneMesh.geometry[lightID];
                        light.face_index.size();
                        std::uniform_int_distribution<int> distrib1(
                                0, light.face_index.size() - 1);

                        // To draw a random variable from the distribution, you use the function call operator()
                        // and pass in an instance of a random number engine, such as a Mersenne Twister.
                        int faceID = distrib1(gen);
                        const auto &v0 = light.vertex[faceID];
                        const auto &v1 = light.vertex[faceID];
                        const auto &v2 = light.vertex[faceID];

                        auto P = v0 + ((v1 - v0) * float(distrib(gen)) +
                                    (v2 - v0) * float(distrib(gen)));

                        // hit is where along the vector it hits the face
                        auto hit = ray.origin +
                                ray.dir * (t - std::numeric_limits<float>::epsilon());
                        auto L = P - hit;

                        auto len = tracer::length(L);

                        // 191 original: check time of flight to see if we have hit occlusion
                        t = len - std::numeric_limits<float>::epsilon();

                        L = tracer::normalize(L);

                        auto mat = SceneMesh.geometry[i].object_material;
                        auto c =
                                (mat.ka * 0.5f + mat.ke) / float(SceneMesh.light_sources.size());

                        if (occlusion(SceneMesh, hit, L, t))
                            continue;

                        auto d = dot(N, L);

                        if (d <= 0)
                            continue;

                        auto H = normalize((N + L) * 2.f);

                        c = c + (mat.kd * d + mat.ks * pow(dot(N, H), mat.Ns)) /
                                float(SceneMesh.light_sources.size());

                        // set pixel on the result image to the calculated color
                        image[h * image_width + w].r += c.r;
                        image[h * image_width + w].g += c.g;
                        image[h * image_width + w].b += c.b;
                    }
                        }
                }

        }

    }
}
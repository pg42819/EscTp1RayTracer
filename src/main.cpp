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

void
scan_row(tracer::scene &SceneMesh, int image_width, int image_height, tracer::camera &cam, tracer::vec3<float> *image,
         std::mt19937 &gen, std::uniform_real_distribution<float> &distrib, int h);


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
    std::string outputname = "output.ppm";
    bool hasEye{false}, hasLook{false};
    bool threaded{false};
    bool flat{false};
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

    // flatten if needed
    if (flat) {
        flatten_scene(SceneMesh);
    }

    // start the clock!
    auto start_time = std::chrono::high_resolution_clock::now();

    // START HERE
    std::vector<std::thread> threads;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distrib(0, 1.f);


    // scan vertically and horizontally for each point in the image window...
    for (int h = image_height - 1; h >= 0; --h) {
        if (threaded) {
            // trace rays on a single row in the image window
            threads.push_back(std::thread(scan_row, std::ref(SceneMesh), image_width, image_height,
                                          std::ref(cam), image, std::ref(gen), std::ref(distrib), h));
        }
        else {
            scan_row(SceneMesh, image_width, image_height, cam, image, gen, distrib, h);
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
    std::cerr << "\n Duration  : "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                                       start_time)
                      .count()
              << std::endl;

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

    std::cout <<   "Rendered image in: " << outputname << std::endl;
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

        // ispc : before we call intersect, convert the scenemesh into an array of triangles

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
    }
}
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <limits>
#include "data_loader.h"
#include "geometry.h"
#include "kdtree.h"

const int WIDTH = 640;
const int HEIGHT = 480;

Vec3 shade(const Vec3& hit_point, const Vec3& normal, const Light& light, Triangle* &hitTriangle) {
    Vec3 l = (light.position - hit_point).normalize();
    float distance_squared = (hit_point - light.position).dot(hit_point - light.position);
    float attenuation = 1.0f / (distance_squared + 1e-4f);
    float cos_theta = std::max(normal.dot(l), 0.0f);
    return (hitTriangle->color * light.color) * hitTriangle->albedo * light.intensity * attenuation * cos_theta;
}

void save(const std::string& filename, const unsigned char* data) {
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
    ofs.write(reinterpret_cast<const char*>(data), WIDTH * HEIGHT * 3);
}

void ray_tracing() {
    unsigned char image[WIDTH * HEIGHT * 3] = {0};
    Vec3 triangle_color(0.0f, 0.01f, 0.0f);
    float triangle_albedo = 1.0f;

    DataLoader loader;
    
    std::vector<float> vertexArray = loader.load("./models/laurel.obj");

    if (vertexArray.empty()) {
        std::cerr << "Failed to load laurel.obj!" << std::endl;
        return;
    }

    std::vector<Triangle> triangles;
    std::vector<Vec3> normals;

    for (size_t i = 0; i < vertexArray.size(); i += 24) {
        Vec3 v0(vertexArray[i], vertexArray[i+1], vertexArray[i+2]);
        Vec3 v1(vertexArray[i+8], vertexArray[i+9], vertexArray[i+10]);
        Vec3 v2(vertexArray[i+16], vertexArray[i+17], vertexArray[i+18]);
        triangles.emplace_back(v0, v1, v2, triangle_color, triangle_albedo);
    }

    KDTree kdtree(triangles);

    Vec3 camera(0, 0, 5);
    Light light(Vec3(0, 0, 5), Vec3(1.0f, 1.0f, 1.0f), 1200.0f);

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            int index = (y * WIDTH + x) * 3;

            float px = (2 * (x + 0.5f) / float(WIDTH) - 1) * WIDTH / float(HEIGHT);
            float py = (1 - 2 * (y + 0.5f) / float(HEIGHT));

            Ray ray(camera, Vec3(px, py, -1));

            float t = std::numeric_limits<float>::max();
            Vec3 color(0, 0, 0);
            Triangle* hitTriangle = nullptr;

            if (kdtree.intersect(ray, t, hitTriangle)) {
                Vec3 hit_point = ray.origin + ray.direction * t;
                Vec3 normal = hitTriangle->getNormal(ray.direction);
                color = shade(hit_point, normal, light, hitTriangle);
            }

            image[index] = static_cast<unsigned char>(std::min(color.x * 255.0f, 255.0f));
            image[index + 1] = static_cast<unsigned char>(std::min(color.y * 255.0f, 255.0f));
            image[index + 2] = static_cast<unsigned char>(std::min(color.z * 255.0f, 255.0f));
        }
    }

    save("./results/laurel.ppm", image);
    std::cout << "Image saved as results/laurel.ppm" << std::endl;
}

int main(){
    ray_tracing();
    return 0;
}
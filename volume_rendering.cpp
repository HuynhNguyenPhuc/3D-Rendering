#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include "geometry.h"

const int WIDTH = 640;
const int HEIGHT = 480;

float transfer_function(float x) {
    return std::min(1 - std::exp(-x), 1.0f);
}

Vec3 shade_sphere(const Vec3& hit_point, const Sphere& sphere, const Vec3& background_color, float travel_distance, float max_travel_distance) {
    // float alpha = std::min(travel_distance / max_travel_distance, 1.0f);
    
    float alpha = (transfer_function(travel_distance / max_travel_distance) - transfer_function(0.0f)) / (transfer_function(1.0f) - transfer_function(0.0f));
    alpha = std::min(std::max(alpha, 0.0f), 1.0f);

    return background_color * (1.0f - alpha) + sphere.color * alpha;
}

void save(const std::string& filename, const unsigned char* data) {
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
    ofs.write(reinterpret_cast<const char*>(data), WIDTH * HEIGHT * 3);
}

void volume_rendering() {
    unsigned char image[WIDTH * HEIGHT * 3] = {0};

    Vec3 camera(0, 0, 0);
    Sphere sphere(Vec3(0, 0, -5), 3.0f, Vec3(0.0f, 0.0f, 0.0f), 1.0f);
    Vec3 background_color(1.0f, 0.0f, 0.0f);

    float max_travel_distance = 2 * sphere.radius;

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            int index = (y * WIDTH + x) * 3;

            float px = (2 * (x + 0.5f) / float(WIDTH) - 1) * WIDTH / float(HEIGHT);
            float py = (1 - 2 * (y + 0.5f) / float(HEIGHT));

            Ray ray(camera, Vec3(px, py, -1).normalize());

            float t0, t1;
            Vec3 color = background_color;

            if (sphere.intersect(ray, t0, t1)) {
                Vec3 hit_point_entrance = ray.origin + ray.direction * t0;
                Vec3 hit_point_exit = ray.origin + ray.direction * t1;

                float travel_distance = (hit_point_exit - hit_point_entrance).length();

                color = shade_sphere(hit_point_entrance, sphere, background_color, travel_distance, max_travel_distance);
            }

            image[index] = static_cast<unsigned char>(std::min(color.x * 255.0f, 255.0f));
            image[index + 1] = static_cast<unsigned char>(std::min(color.y * 255.0f, 255.0f));
            image[index + 2] = static_cast<unsigned char>(std::min(color.z * 255.0f, 255.0f));
        }
    }

    save("./results/volume_rendering.ppm", image);
    std::cout << "Image saved as results/volume_rendering.ppm" << std::endl;
}

int main() {
    volume_rendering();
    return 0;
}
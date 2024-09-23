#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include "geometry.h"

const int WIDTH = 640;
const int HEIGHT = 480;
const Vec3 BACKGROUND_COLOR(1.0f, 0.0f, 0.0f);
const float SIGMA_A = 0.35f;
const int NUM_STEPS = 10;

float transfer_function(float distance) {
    return std::max(0.0f, std::min(std::exp(-distance * SIGMA_A), 1.0f));
}

Vec3 get_light_intensity(const Vec3& particle_hit_point, const Sphere& sphere, const Light& light, float step_size) {
    Vec3 light_dir = (particle_hit_point - light.position).normalize();
    Ray ray(light.position, light_dir);
    float t;
    if (sphere.intersect(ray, t)){
        Vec3 sphere_hit_point = ray.position(t);
        float light_transfer_distance = (particle_hit_point - sphere_hit_point).length();
        return light.color * step_size * transfer_function(light_transfer_distance);
    }
    return Vec3();
}

Vec3 shade_sphere(const Vec3& hit_point_entrance, const Vec3& hit_point_exit, const Sphere& sphere, const Light& light) {
    Vec3 result(0.0f, 0.0f, 0.0f);
    float transmission = 1.0f;

    float step_size = (hit_point_exit - hit_point_entrance).length() / NUM_STEPS;
    Vec3 step_direction = (hit_point_exit - hit_point_entrance) / (float) NUM_STEPS;
    Vec3 current_point = hit_point_entrance + step_direction * 0.5;

    float attenuation = transfer_function(step_size); // Beer's law for light attenuation

    for (int i = 0; i < NUM_STEPS; ++i) {
        Vec3 L_i_x_i = get_light_intensity(current_point, sphere, light, step_size);
        transmission *= attenuation;
        result += L_i_x_i * transmission;
        current_point += step_direction;
    }

    return BACKGROUND_COLOR * transmission + result;
}

void save(const std::string& filename, const unsigned char* data) {
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
    ofs.write(reinterpret_cast<const char*>(data), WIDTH * HEIGHT * 3);
}

void forward_ray_marching() {
    unsigned char image[WIDTH * HEIGHT * 3] = {0};

    Vec3 camera(0, 0, 0);
    Sphere sphere(Vec3(0, 0, -5), 3.0f, Vec3(0.0f, 0.0f, 0.0f), SIGMA_A);
    Light light(Vec3(2, 2, -7), Vec3(1.0f, 1.0f, 1.0f), 10.0f);

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            int index = (y * WIDTH + x) * 3;

            float px = (2 * (x + 0.5f) / float(WIDTH) - 1) * WIDTH / float(HEIGHT);
            float py = (1 - 2 * (y + 0.5f) / float(HEIGHT));

            Ray ray(camera, Vec3(px, py, -1).normalize());

            float t0, t1;
            Vec3 color = BACKGROUND_COLOR;

            if (sphere.intersect(ray, t0, t1)) {
                Vec3 hit_point_entrance = ray.position(t0);
                Vec3 hit_point_exit = ray.position(t1);

                color = shade_sphere(hit_point_entrance, hit_point_exit, sphere, light);
            }

            image[index] = static_cast<unsigned char>(std::min(color.x * 255.0f, 255.0f));
            image[index + 1] = static_cast<unsigned char>(std::min(color.y * 255.0f, 255.0f));
            image[index + 2] = static_cast<unsigned char>(std::min(color.z * 255.0f, 255.0f));
        }
    }

    save("./results/forward_ray_marching.ppm", image);
    std::cout << "Image saved as results/forward_ray_marching.ppm" << std::endl;
}

int main() {
    forward_ray_marching();
    return 0;
}
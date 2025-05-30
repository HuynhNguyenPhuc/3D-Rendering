#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <sstream>
#include "geometry.h"

const Vec3 BACKGROUND_COLOR(0.572f, 0.772f, 0.921f);

float transfer_function(float distance, float sigma_a) {
    return std::clamp(std::exp(-distance * sigma_a), 0.0f, 1.0f);
}

Vec3 get_light_intensity(const Vec3& particle_hit_point, const Sphere& sphere, const Light& light, float step_size, float sigma_a) {
    Vec3 light_dir = (particle_hit_point - light.position).normalize();
    Ray ray(light.position, light_dir);
    float t;
    if (sphere.intersect(ray, t)) {
        Vec3 sphere_hit_point = ray.position(t);
        float light_transfer_distance = (particle_hit_point - sphere_hit_point).length();
        return light.color * step_size * transfer_function(light_transfer_distance, sigma_a);
    }
    return Vec3();
}

Vec3 shade_sphere(const Vec3& hit_point_entrance, const Vec3& hit_point_exit, const Sphere& sphere, const Light& light, float sigma_a, int num_steps) {
    Vec3 result = sphere.material.color;
    float transmission = 1.0f;

    float step_size = (hit_point_entrance - hit_point_exit).length() / num_steps;
    Vec3 step_direction = (hit_point_entrance - hit_point_exit) / (float) num_steps;
    Vec3 current_point = hit_point_exit + step_direction * 0.5;

    float attenuation = transfer_function(step_size, sigma_a); // Beer's law for light attenuation

    for (int i = 0; i < num_steps; ++i) {
        Vec3 L_i_x_i = get_light_intensity(current_point, sphere, light, step_size, sigma_a);
        transmission *= attenuation;
        result = (result + L_i_x_i) * attenuation;
        current_point += step_direction;
    }

    return BACKGROUND_COLOR * transmission + result;
}

void backward_ray_marching(int width, int height, float sigma_a, int num_steps) {
    std::vector<unsigned char> image(width * height * 3, 0);

    Vec3 camera(0.0f, 0.0f, 0.0f);
    Material material(Vec3(0.0f), Vec3(1.0f), 0.1f, 0.9f, 0.5f, 1.0f, 0.0f, 32.0f, MaterialType::NONE);
    Sphere sphere(Vec3(0.0f, 0.0f, -5.0f), 3.0f, material);
    Light light(Vec3(4.0f, 4.0f, -7.0f), Vec3(1.3f, 0.3f, 0.9f), 10.0f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float u = (2.0f * (x + 0.5f) / width - 1.0f) * (static_cast<float>(width) / height);
            float v = (1.0f - 2.0f * (y + 0.5f) / height);
            Ray ray(camera, Vec3(u, v, -1.0f).normalize());
            Vec3 color = BACKGROUND_COLOR;

            float t0, t1;
            if (sphere.intersect(ray, t0, t1)) {
                Vec3 hit_point_entrance = ray.position(t0);
                Vec3 hit_point_exit = ray.position(t1);
                color = shade_sphere(hit_point_entrance, hit_point_exit, sphere, light, sigma_a, num_steps);
            }

            int idx = 3 * (y * width + x);
            image[idx]     = static_cast<unsigned char>(std::min(color.x * 255.0f, 255.0f));
            image[idx + 1] = static_cast<unsigned char>(std::min(color.y * 255.0f, 255.0f));
            image[idx + 2] = static_cast<unsigned char>(std::min(color.z * 255.0f, 255.0f));
        }
    }

    save_png("./results/backward_ray_marching.png", image.data(), width, height);
    std::cout << "Saved backward_ray_marching.png successfully" << std::endl;
}

int main(int argc, char** argv) {
    int width = 640, height = 480;
    float sigma_a = 0.45f;
    int steps = 10;

    for (int i = 1; i < argc; i += 2) {
        std::string arg = (i + 1 < argc) ? argv[i] : "";
        if (arg == "-w" || arg == "--width") width = std::stoi(argv[i + 1]);
        else if (arg == "-h" || arg == "--height") height = std::stoi(argv[i + 1]);
        else if (arg == "-s" || arg == "--sigma") sigma_a = std::stof(argv[i + 1]);
        else if (arg == "-n" || arg == "--steps") steps = std::stoi(argv[i + 1]);
    }

    if (width <= 0 || height <= 0) {
        std::cerr << "Invalid width or height value" << std::endl;
        return 1;
    }
    if (sigma_a < 0.0f) sigma_a = 0.0f;
    if (steps <= 0) steps = 1;

    std::cout << "Backward Ray Marching: " << width << "x" << height
              << ", sigma_a=" << sigma_a << ", steps=" << steps << std::endl;

    backward_ray_marching(width, height, sigma_a, steps);
    return 0;
}
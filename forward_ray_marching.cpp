#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <sstream>
#include "geometry.h"

const Vec3 BACKGROUND_COLOR(0.572f, 0.772f, 0.921f);

float transfer_function(float distance, float sigma_a) {
    return std::max(0.0f, std::min(std::exp(-distance * sigma_a), 1.0f));
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

    float step_size = (hit_point_exit - hit_point_entrance).length() / num_steps;
    Vec3 step_direction = (hit_point_exit - hit_point_entrance) / float(num_steps);
    Vec3 current_point = hit_point_entrance + step_direction * 0.5f;

    float attenuation = transfer_function(step_size, sigma_a);

    for (int i = 0; i < num_steps; ++i) {
        Vec3 L_i_x_i = get_light_intensity(current_point, sphere, light, step_size, sigma_a);
        transmission *= attenuation;
        result += L_i_x_i * transmission;
        current_point += step_direction;
    }

    return BACKGROUND_COLOR * transmission + result;
}

void forward_ray_marching(int width, int height, float sigma_a, int num_steps) {
    std::vector<unsigned char> image(width * height * 3, 0);

    Vec3 camera(0.0f, 0.0f, 0.0f);
    Material material(Vec3(0.0f), Vec3(1.0f), 0.1f, 0.9f, 0.5f, 1.0f, 0.0f, 32.0f, MaterialType::NONE);
    Sphere sphere(Vec3(0.0f, 0.0f, -5.0f), 3.0f, material);
    Light light(Vec3(4.0f, 4.0f, -7.0f), Vec3(1.3f, 0.3f, 0.9f), 10.0f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3;

            float px = (2 * (x + 0.5f) / float(width) - 1) * width / float(height);
            float py = (1 - 2 * (y + 0.5f) / float(height));

            Ray ray(camera, Vec3(px, py, -1).normalize());

            float t0, t1;
            Vec3 color = BACKGROUND_COLOR;

            if (sphere.intersect(ray, t0, t1)) {
                Vec3 hit_point_entrance = ray.position(t0);
                Vec3 hit_point_exit = ray.position(t1);
                color = shade_sphere(hit_point_entrance, hit_point_exit, sphere, light, sigma_a, num_steps);
            }

            image[index] = static_cast<unsigned char>(std::min(color.x * 255.0f, 255.0f));
            image[index + 1] = static_cast<unsigned char>(std::min(color.y * 255.0f, 255.0f));
            image[index + 2] = static_cast<unsigned char>(std::min(color.z * 255.0f, 255.0f));
        }
    }

    save_png("./results/forward_ray_marching.png", image.data(), width, height);
    std::cout << "Image saved as results/forward_ray_marching.png" << std::endl;
}

int main(int argc, char** argv) {
    int width = 640;
    int height = 480;
    float sigma_a = 0.35f;
    int num_steps = 10;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--width" || arg == "-w") && i + 1 < argc) {
            width = std::stoi(argv[++i]);
        } else if ((arg == "--height" || arg == "-h") && i + 1 < argc) {
            height = std::stoi(argv[++i]);
        } else if ((arg == "--sigma" || arg == "-s") && i + 1 < argc) {
            sigma_a = std::stof(argv[++i]);
        } else if ((arg == "--steps" || arg == "-n") && i + 1 < argc) {
            num_steps = std::stoi(argv[++i]);
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            std::exit(1);
        }
    }

    if (width <= 0 || height <= 0) {
        std::cerr << "Invalid width or height value" << std::endl;
        return 1;
    }
    if (sigma_a < 0.0f) sigma_a = 0.0f;
    if (num_steps <= 0) num_steps = 1;

    std::cout << "Forward Ray Marching: " << width << "x" << height
              << ", sigma_a=" << sigma_a << ", steps=" << num_steps << std::endl;

    forward_ray_marching(width, height, sigma_a, num_steps);
    return 0;
}
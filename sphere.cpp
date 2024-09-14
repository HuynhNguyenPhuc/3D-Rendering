#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include "geometry.h"

const int WIDTH = 640;
const int HEIGHT = 480;

// Function to calculate lighting based on sphere
Vec3 shade_sphere(const Vec3& hit_point, const Vec3& normal, const Light& light, const Sphere& sphere) {
    Vec3 l = (light.position - hit_point).normalize();
    float distance_squared = (hit_point - light.position).dot(hit_point - light.position);
    float attenuation = 1.0f / (distance_squared + 1e-4f);
    float cos_theta = std::max(normal.dot(l), 0.0f);
    return (sphere.color * light.color) * sphere.albedo * light.intensity * attenuation * cos_theta;
}

Vec3 shade_plane(const Vec3& hit_point, const Vec3& normal, const Light& light, const Plane& plane, const Sphere& sphere) {
    Vec3 l = (light.position - hit_point).normalize();
    float distance_squared = (hit_point - light.position).dot(hit_point - light.position);
    float attenuation = 1.0f / (distance_squared + 1e-4f);
    float cos_theta = std::max(normal.dot(l), 0.0f);

    float t_sphere;
    bool in_shadow = false;
    if (sphere.intersect(Ray(hit_point, l), t_sphere)) {
        Vec3 shadow_hit_point = hit_point + l * t_sphere;
        if ((shadow_hit_point - light.position).dot(shadow_hit_point - light.position) < distance_squared) {
            in_shadow = true;
        }
    }

    Vec3 diffuse = (plane.color * light.color) * plane.albedo * light.intensity * attenuation * cos_theta;
    if (in_shadow) {
        diffuse = diffuse * 0.3f;
    }
    return diffuse;
}

void save(const std::string& filename, const unsigned char* data) {
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
    ofs.write(reinterpret_cast<const char*>(data), WIDTH * HEIGHT * 3);
}

void ray_tracing() {
    unsigned char image[WIDTH * HEIGHT * 3] = {0};

    Vec3 camera(0, 0, 0);
    Sphere sphere(Vec3(0, 0, -5), 2.0f, Vec3(0.25f, 1.0f, 0.25f), 1.0f);
    Plane plane(Vec3(0, 1, 0), 3.0f, Vec3(0.5f, 0.5f, 1.0f), 1.0f);
    Light light(Vec3(5, 5, 4), Vec3(1.0f, 1.0f, 1.0f), 120.0f);

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            int index = (y * WIDTH + x) * 3;

            float px = (2 * (x + 0.5f) / float(WIDTH) - 1) * WIDTH / float(HEIGHT);
            float py = (1 - 2 * (y + 0.5f) / float(HEIGHT));

            Ray ray(camera, Vec3(px, py, -1));

            float t;
            Vec3 color(0, 0, 0);

            if (sphere.intersect(ray, t)) {
                Vec3 hit_point = ray.origin + ray.direction * t;
                Vec3 normal = (hit_point - sphere.center).normalize();
                color = shade_sphere(hit_point, normal, light, sphere);
            } else if (plane.intersect(ray, t)) {
                Vec3 hit_point = ray.origin + ray.direction * t;
                Vec3 normal = plane.normal;
                color = shade_plane(hit_point, normal, light, plane, sphere);
            }

            image[index] = static_cast<unsigned char>(std::min(color.x * 255.0f, 255.0f));
            image[index + 1] = static_cast<unsigned char>(std::min(color.y * 255.0f, 255.0f));
            image[index + 2] = static_cast<unsigned char>(std::min(color.z * 255.0f, 255.0f));
        }
    }

    save("./results/sphere.ppm", image);
    std::cout << "Image saved as results/sphere.ppm" << std::endl;
}

int main() {
    ray_tracing();
    return 0;
}
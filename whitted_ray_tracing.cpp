/*
 * Whitted Ray Tracing algorithm
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <memory>
#include "vec3.h"
#include "material.h"
#include "geometry.h"
#include "optics.h"
#include "primitive_tree.h"

const int WIDTH = 920;
const int HEIGHT = 640;
const int MAX_BOUNCES = 50;
const Vec3 BACKGROUND_COLOR = Vec3(0.0f, 0.0f, 0.0f);

Vec3 cast_ray(const Ray& ray, const PrimitiveTree& primitives, const std::vector<Light*>& lights, int depth) {
    if (depth > MAX_BOUNCES) return BACKGROUND_COLOR;

    float t;
    Primitive* hitPrimitive;

    if (!primitives.intersect(ray, t, hitPrimitive)) {
        return BACKGROUND_COLOR;
    }

    Vec3 hit_point = ray.position(t);
    Vec3 normal = hitPrimitive->getNormal(hit_point);
    Vec3 color(0.0f, 0.0f, 0.0f);

    switch (hitPrimitive->material.type) {
        case REFRACTIVE: {
            // Compute reflection direction
            Vec3 reflected_direction = reflection(ray.direction, normal);
            Ray reflected_ray(hit_point + normal * 1e-3, reflected_direction);
            Vec3 reflected_color = cast_ray(reflected_ray, primitives, lights, depth + 1);

            // Compute refraction direction
            bool isInside = false;
            Vec3 refracted_direction = refraction(ray.direction, normal, hitPrimitive->material.ior, isInside);
            Vec3 refracted_color(0.0f);
            if (refracted_direction != Vec3(0.0f)) {
                Ray refracted_ray(hit_point + normal * (2*(int) isInside - 1) * 1e-3, refracted_direction);
                refracted_color = cast_ray(refracted_ray, primitives, lights, depth + 1);
            }

            // Compute Fresnel effect
            float kr = fresnel(ray.direction, normal, hitPrimitive->material.ior);
            color = reflected_color * kr + refracted_color * (1 - kr);
            break;
        }
        case REFLECTIVE: {
            Vec3 reflected_direction = reflection(ray.direction, normal);
            Ray reflected_ray(hit_point + normal * 1e-3, reflected_direction);
            color = cast_ray(reflected_ray, primitives, lights, depth + 1);
            break;
        }
        case NONE: {
            for (const auto& light : lights) {
                Vec3 light_direction = (light->position - hit_point).normalize();
                float light_intensity = light->intensity;

                float light_distance_2 = (light->position - hit_point).dot(light->position - hit_point);

                Ray shadow_ray(hit_point + normal * 1e-3, light_direction);
                float tShadow;
                Primitive* shadowHitPrimitive;
                bool isInShadow = primitives.intersect(shadow_ray, tShadow, shadowHitPrimitive) && tShadow * tShadow < light_distance_2;

                if (!isInShadow) {
                    Vec3 diffuse = hitPrimitive->material.color * hitPrimitive->material.kD * light_intensity * std::max(0.0f, light_direction.dot(normal));
                    Vec3 reflected_direction = (ray.direction - normal * ray.direction.dot(normal) * 2).normalize();
                    Vec3 specular = Vec3(1.0f) * hitPrimitive->material.kS * light_intensity * std::pow(std::max(0.0f, reflected_direction.dot(-light_direction)), hitPrimitive->material.shininess);
                    color += diffuse + specular;
                }
            }
            break;
        }
        default:
            break;
    }

    return color;
}

void save(const std::string& filename, const unsigned char* data) {
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
    ofs.write(reinterpret_cast<const char*>(data), WIDTH * HEIGHT * 3);
}

void whitted_ray_tracing() {
    unsigned char image[WIDTH * HEIGHT * 3] = {0};

    Vec3 camera(0.0f, 0.0f, 0.0f);

    std::vector<Primitive*> primitivesList;

    std::vector<Vec3> colors = {
        Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(1.0f, 1.0f, 0.0f),
        Vec3(1.0f, 0.0f, 1.0f), Vec3(0.0f, 1.0f, 1.0f), Vec3(0.5f, 0.5f, 0.5f), Vec3(1.0f, 0.5f, 0.0f),
        Vec3(0.5f, 0.0f, 1.0f), Vec3(0.0f, 0.5f, 1.0f), Vec3(1.0f, 0.5f, 0.5f), Vec3(0.5f, 1.0f, 0.5f),
        Vec3(0.5f, 0.5f, 1.0f), Vec3(1.0f, 1.0f, 1.0f), Vec3(0.8f, 0.8f, 0.8f), Vec3(0.3f, 0.7f, 0.4f)
    };

    std::vector<float> radii = {0.7f, 0.8f, 0.9f, 1.0f, 0.6f, 0.9f, 0.7f, 0.8f, 1.0f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 0.7f, 0.8f};

    std::vector<MaterialType> materialTypes = {
        MaterialType::REFLECTIVE, MaterialType::REFRACTIVE, MaterialType::NONE, MaterialType::REFLECTIVE,
        MaterialType::REFRACTIVE, MaterialType::NONE, MaterialType::REFLECTIVE, MaterialType::REFRACTIVE,
        MaterialType::NONE, MaterialType::REFLECTIVE, MaterialType::REFRACTIVE, MaterialType::NONE,
        MaterialType::REFLECTIVE, MaterialType::REFRACTIVE, MaterialType::NONE, MaterialType::REFLECTIVE
    };

    float spacing = 2.2f;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            int index = i * 4 + j;
            Vec3 color = colors[index];
            float radius = radii[index];
            MaterialType matType = materialTypes[index];
            Material material(color, 0.6f, 0.3f, 0.5f, 0.5f, (matType == MaterialType::REFRACTIVE ? 0.8f : 0.0f), 1.5f, 32.0f, matType);

            Vec3 position(-3.5f + j * spacing, -1.5f, -8.0f + i * spacing);
            primitivesList.push_back(new Sphere(position, radius, material));
        }
    }

    Material groundMaterial(Vec3(0.5f, 0.5f, 0.5f), 0.6f, 0.3f, 0.5f, 0.5f, 0.0f, 1.0f, 16.0f, MaterialType::NONE);
    primitivesList.push_back(new Plane(Vec3(0.0f, 0.75f, 0.0f), 2.0f, groundMaterial));

    PrimitiveTree primitives(primitivesList);

    std::vector<Light*> lights;
    lights.push_back(new Light(Vec3(-10.0f, 10.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f), 1.0f));
    lights.push_back(new Light(Vec3(10.0f, 10.0f, 10.0f), Vec3(0.8f, 0.8f, 0.8f), 0.6f));

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            int index = (y * WIDTH + x) * 3;

            float px = (2 * (x + 0.5f) / float(WIDTH) - 1) * WIDTH / float(HEIGHT);
            float py = (1 - 2 * (y + 0.5f) / float(HEIGHT));

            Ray ray(camera, Vec3(px, py, -1).normalize());
            Vec3 color = cast_ray(ray, primitives, lights, 0);

            image[index] = static_cast<unsigned char>(std::min(color.x * 255.0f, 255.0f));
            image[index + 1] = static_cast<unsigned char>(std::min(color.y * 255.0f, 255.0f));
            image[index + 2] = static_cast<unsigned char>(std::min(color.z * 255.0f, 255.0f));
        }
    }

    for (Primitive* primitive : primitivesList) {
        delete primitive;
    }

    for (Light* light : lights) {
        delete light;
    }

    save("./results/whitted_ray_tracing.ppm", image);
    std::cout << "Image saved as results/whitted_ray_tracing.ppm" << std::endl;
}

int main() {
    whitted_ray_tracing();
    return 0;
}
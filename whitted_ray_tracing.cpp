#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <memory>
#include <cstring>
#include "vec3.h"
#include "material.h"
#include "geometry.h"
#include "optics.h"
#include "primitive_tree.h"

Vec3 cast_ray(const Ray& ray, const PrimitiveTree& primitives, const std::vector<Light*>& lights, int depth, int max_bounces, const Vec3& background_color) {
    if (depth > max_bounces) return background_color;

    float t;
    Primitive* hitPrimitive;

    if (!primitives.intersect(ray, t, hitPrimitive)) {
        return background_color;
    }

    Vec3 hit_point = ray.position(t);
    Vec3 normal = hitPrimitive->getNormal(hit_point);
    Vec3 color(0.0f, 0.0f, 0.0f);

    switch (hitPrimitive->material.type) {
        case REFRACTIVE: {
            Vec3 reflected_direction = reflection(ray.direction, normal);
            Ray reflected_ray(hit_point + normal * 1e-3, reflected_direction);
            Vec3 reflected_color = cast_ray(reflected_ray, primitives, lights, depth + 1, max_bounces, background_color);

            bool isInside = false;
            Vec3 refracted_direction = refraction(ray.direction, normal, hitPrimitive->material.ior, isInside);
            Vec3 refracted_color(0.0f);
            if (refracted_direction != Vec3(0.0f)) {
                Ray refracted_ray(hit_point + normal * (2*(int)isInside - 1) * 1e-3, refracted_direction);
                refracted_color = cast_ray(refracted_ray, primitives, lights, depth + 1, max_bounces, background_color);
            }

            float kr = fresnel(ray.direction, normal, hitPrimitive->material.ior);
            color = reflected_color * kr + refracted_color * (1 - kr);
            break;
        }
        case REFLECTIVE: {
            Vec3 reflected_direction = reflection(ray.direction, normal);
            Ray reflected_ray(hit_point + normal * 1e-3, reflected_direction);
            color = cast_ray(reflected_ray, primitives, lights, depth + 1, max_bounces, background_color);
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

void whitted_ray_tracing(int width, int height, int max_bounces, const std::string& output_path, const Vec3& background_color) {
    unsigned char* image = new unsigned char[width * height * 3]();

    Vec3 camera(0.0f, 0.0f, 2.0f);

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
            Material material(color, Vec3(1.0f), 0.3f, 0.5f, 0.5f, (matType == MaterialType::REFRACTIVE ? 0.8f : 0.0f), 1.5f, 32.0f, matType);

            Vec3 position(-3.5f + j * spacing, -1.5f, -8.0f + i * spacing);
            primitivesList.push_back(new Sphere(position, radius, material));
        }
    }

    Material groundMaterial(Vec3(0.5f, 0.5f, 0.5f), Vec3(1.0f), 0.3f, 0.5f, 0.5f, 0.0f, 1.0f, 16.0f, MaterialType::NONE);
    primitivesList.push_back(new Plane(Vec3(0.0f, 0.75f, 0.0f), 2.0f, groundMaterial));

    PrimitiveTree primitives(primitivesList);

    std::vector<Light*> lights;
    lights.push_back(new Light(Vec3(0.0f, 0.0f, 5.0f), Vec3(0.0f, 0.0f, -1.0f), 2.0f));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3;

            float px = (2 * (x + 0.5f) / float(width) - 1) * width / float(height);
            float py = (1 - 2 * (y + 0.5f) / float(height));

            Ray ray(camera, Vec3(px, py, -1).normalize());
            Vec3 color = cast_ray(ray, primitives, lights, 0, max_bounces, background_color);

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

    save_png(output_path.c_str(), image, width, height);
    std::cout << "Image saved as " << output_path << std::endl;

    delete[] image;
}

int main(int argc, char* argv[]) {
    int width = 1280;
    int height = 1024;
    int max_bounces = 50;
    std::string output_path = "./results/whitted_ray_tracing.png";

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
            width = std::atoi(argv[++i]);
        } else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
            height = std::atoi(argv[++i]);
        } else if (strcmp(argv[i], "--max-bounces") == 0 && i + 1 < argc) {
            max_bounces = std::atoi(argv[++i]);
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            output_path = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            std::cout << "Usage: " << argv[0] << " [--width W] [--height H] [--max-bounces M] [--output PATH]\n"
                      << "  --width       Image width in pixels (default: 1280)\n"
                      << "  --height      Image height in pixels (default: 1024)\n"
                      << "  --max-bounces Maximum number of ray bounces (default: 50)\n"
                      << "  --output      Output PNG file path (default: ./results/whitted_ray_tracing.png)\n";
            return 0;
        } else {
            std::cerr << "Unknown or incomplete argument: " << argv[i] << "\n";
            std::cerr << "Use --help for usage information.\n";
            return 1;
        }
    }

    if (width <= 0 || height <= 0 || max_bounces < 0) {
        std::cerr << "Invalid arguments: width and height must be positive, max-bounces must be non-negative.\n";
        return 1;
    }

    Vec3 background_color(0.0f, 0.0f, 0.0f);
    whitted_ray_tracing(width, height, max_bounces, output_path, background_color);

    return 0;
}
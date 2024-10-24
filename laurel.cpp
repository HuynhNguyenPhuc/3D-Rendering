#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <limits>
#include "vec3.h"
#include "data_loader.h"
#include "geometry.h"
#include "primitive_tree.h"
#include "optics.h"
#include "material.h"

const int WIDTH = 640;
const int HEIGHT = 480;
const int MAX_BOUNCES = 10;
const Vec3 BACKGROUND_COLOR = Vec3(0.0f, 0.0f, 0.0f);

Vec3 cast_ray(const Ray& ray, const PrimitiveTree& primitives, const std::vector<Light*>& lights, int depth) {
    if (depth > MAX_BOUNCES) return BACKGROUND_COLOR;

    float t;
    Primitive* hitprimitive;

    if (!primitives.intersect(ray, t, hitprimitive)) {
        return BACKGROUND_COLOR;
    }

    Vec3 hit_point = ray.position(t);
    Vec3 normal = hitprimitive->getNormal(hit_point);
    Vec3 color(0.0f, 0.0f, 0.0f);

    switch (hitprimitive->material.type) {
        case REFRACTIVE: {
            Vec3 reflected_direction = reflection(ray.direction, normal);
            Ray reflected_ray(hit_point + normal * 1e-3, reflected_direction);
            Vec3 reflected_color = cast_ray(reflected_ray, primitives, lights, depth + 1);

            Vec3 refracted_direction = refraction(ray.direction, normal, hitprimitive->material.ior);
            Vec3 refracted_color(0.0f);
            if (refracted_direction != Vec3(0.0f)) {
                Ray refracted_ray(hit_point + normal * (2*(int) isInside - 1) * 1e-3, refracted_direction);
                refracted_color = cast_ray(refracted_ray, primitives, lights, depth + 1);
            }

            float kr = fresnel(ray.direction, normal, hitprimitive->material.ior);
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
                Primitive* shadowHitprimitive;
                bool isInShadow = primitives.intersect(shadow_ray, tShadow, shadowHitprimitive) && tShadow * tShadow < light_distance_2;

                if (!isInShadow) {
                    Vec3 diffuse = hitprimitive->material.color * hitprimitive->material.kD * std::max(0.0f, light_direction.dot(normal)) * light_intensity;
                    Vec3 reflected_direction = reflection(ray.direction, normal);
                    Vec3 specular = Vec3(1.0f) * hitprimitive->material.kS * std::pow(std::max(0.0f, reflected_direction.dot(-light_direction)), hitprimitive->material.shininess) * light_intensity;
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

    DataLoader loader;
    std::vector<float> vertexArray = loader.load("./models/laurel.obj");

    if (vertexArray.empty()) {
        std::cerr << "Failed to load laurel.obj!" << std::endl;
        return;
    }

    std::vector<Primitive*> primitive_pointers;

    Vec3 primitive_color(0.0f, 0.5f, 0.0f);
    Material material(primitive_color, 0.6f, 0.3f, 0.5f, 0.5f, 0.0f, 1.0f, 16.0f, MaterialType::NONE); // Set as REFRACTIVE

    for (size_t i = 0; i < vertexArray.size(); i += 24) {
        Vec3 v0(vertexArray[i], vertexArray[i+1], vertexArray[i+2]);
        Vec3 v1(vertexArray[i+8], vertexArray[i+9], vertexArray[i+10]);
        Vec3 v2(vertexArray[i+16], vertexArray[i+17], vertexArray[i+18]);
        primitive_pointers.push_back(new Triangle(v0, v1, v2, material));
    }

    Material groundMaterial(Vec3(0.5f, 0.5f, 0.5f), 0.6f, 0.3f, 0.5f, 0.5f, 0.0f, 1.0f, 16.0f, MaterialType::NONE);
    primitive_pointers.push_back(new Plane(Vec3(0.0f, 0.75f, 0.0f), 2.0f, groundMaterial));

    PrimitiveTree primitives(primitive_pointers);
    
    Vec3 camera(0, 0, 5);
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

    for (Primitive* primitive : primitive_pointers){
        delete primitive;
    }

    for (Light* light : lights) {
        delete light;
    }

    save("./results/laurel.ppm", image);
    std::cout << "Image saved as results/laurel.ppm" << std::endl;
}

int main() {
    whitted_ray_tracing();
    return 0;
}
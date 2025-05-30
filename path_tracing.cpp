/*
 * Path Tracing algorithm (Global Illumination)
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <memory>
#include <random>
#include <cstring>
#include "vec3.h"
#include "material.h"
#include "geometry.h"
#include "optics.h"
#include "primitive_tree.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const Vec3 BACKGROUND_COLOR = Vec3(0.0f, 0.0f, 0.0f);
const float EPSILON = 1e-4f;

std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0, 1);

void create_coordinate_system(const Vec3& N, Vec3& N_t, Vec3& N_b){
    // Find the tagent vector
    if (std::abs(N.x) > std::abs(N.y)) N_t = Vec3(N.z, 0, -N.x);
    else N_t = Vec3(0, -N.z, N.y);
    N_t = N_t.normalize();

    // Find the bitangent vector
    N_b = N.cross(N_t);
}

Vec3 uniform_sample_hemisphere(float &r1, float &r2){
    float sinTheta = std::sqrt(1 - r1 * r1);
    float phi = 2 * M_PI * r2;
    float x = sinTheta * std::cos(phi);
    float z = sinTheta * std::sin(phi);
    return Vec3(x, r1, z);
}

Vec3 cast_ray(
    const Ray& ray,
    const PrimitiveTree& primitives,
    const std::vector<Light*>& lights,
    int depth,
    int max_bounces,
    int num_samples
){
    if (depth > max_bounces) return BACKGROUND_COLOR;

    float t;
    Primitive* hitPrimitive;
    if (!primitives.intersect(ray, t, hitPrimitive))
        return BACKGROUND_COLOR;

    Vec3 hit_point = ray.position(t);
    Vec3 N         = hitPrimitive->getNormal(hit_point);
    auto& M        = hitPrimitive->material;

    // Pre-compute BRDF and pdf for hemisphere (BRDF) sampling
    Vec3 brdf      = M.color * M.kD / M_PI;
    float pdf_brdf = 1.0f / (2.0f * M_PI);

    // --- 1) Direct lighting estimate (light sampling only) ---
    Vec3 Ld(0.0f);
    for (auto* light : lights) {
        Vec3 toLight = light->position - hit_point;
        float dist2  = toLight.dot(toLight);
        float dist   = std::sqrt(dist2);
        Vec3 wi      = toLight / dist;

        // Shadow ray
        Ray shadow(hit_point + N * EPSILON, wi);
        float tS; Primitive* blk;
        if (primitives.intersect(shadow, tS, blk) && tS < dist) 
            continue;

        float cosTheta = std::max(0.0f, N.dot(wi));
        Vec3 Li        = light->color * light->intensity / dist2;
        // pdf_light = 1 for point light
        Ld += brdf * Li * cosTheta;
    }

    // --- 2) Indirect lighting estimate (BRDF sampling only) ---
    Vec3 Li_sum(0.0f);
    Vec3 Nt, Nb;
    create_coordinate_system(N, Nt, Nb);
    for (int i = 0; i < num_samples; ++i) {
        float r1 = distribution(generator);
        float r2 = distribution(generator);
        Vec3 samp = uniform_sample_hemisphere(r1, r2);
        Vec3 wi   = (Nb * samp.x + N * samp.y + Nt * samp.z).normalize();
        float cosTheta = std::max(0.0f, N.dot(wi));

        Ray indirect(hit_point + wi * EPSILON, wi);
        Vec3 Li = cast_ray(indirect, primitives, lights, depth + 1, max_bounces, num_samples);
        Li_sum += Li * brdf * cosTheta / pdf_brdf;
    }
    Vec3 Li_indirect = Li_sum / float(num_samples);

    // --- 3) MIS weighting and final combine ---
    // pdf_light = 1.0f for point light sampling
    float pdf_light = 1.0f; 
    float w_light = pdf_light / (pdf_light + pdf_brdf);
    float w_brdf  = pdf_brdf  / (pdf_light + pdf_brdf);

    Vec3 L = Ld * w_light + Li_indirect * w_brdf;
    return L;
}


void path_tracing(int width, int height, int max_bounces, int num_samples, const std::string& output_path) {
    unsigned char* image = new unsigned char[width * height * 3]();

    Vec3 camera(0.0f, 0.0f, 3.0f);

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
            Material material(color, Vec3(0.188559, 0.287, 0.200726), 0.3f, 0.5f, 0.5f, (matType == MaterialType::REFRACTIVE ? 0.8f : 0.0f), 1.5f, 32.0f, matType);

            Vec3 position(-3.5f + j * spacing, -1.5f, -8.0f + i * spacing);
            primitivesList.push_back(new Sphere(position, radius, material));
        }
    }

    Material groundMaterial(Vec3(0.5f, 0.5f, 0.5f), Vec3(0.225, 0.144, 0.144), 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 16.0f, MaterialType::NONE);
    primitivesList.push_back(new Plane(Vec3(0.0f, 0.75f, 0.0f), 2.0f, groundMaterial));

    PrimitiveTree primitives(primitivesList);

    std::vector<Light*> lights;
    lights.push_back(new Light(Vec3(0.0f, 10.0f, 10.0f), Vec3(1.0f, 1.0f, 1.0f), 1000.0f));
    lights.push_back(new Light(Vec3(0.0f, 10.0f, -10.0f), Vec3(1.0f, 1.0f, 1.0f), 1000.0f));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3;

            float px = (2 * (x + 0.5f) / float(width) - 1) * width / float(height);
            float py = (1 - 2 * (y + 0.5f) / float(height));

            Ray ray(camera, Vec3(px, py, -1).normalize());
            Vec3 color = cast_ray(ray, primitives, lights, 0, max_bounces, num_samples);

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
    std::cout << "Image saved as " << output_path.c_str() << std::endl;

    delete[] image;
}

int main(int argc, char* argv[]) {
    int width = 1280;
    int height = 1024;
    int max_bounces = 2;
    int num_samples = 100;
    std::string output_path = "./results/path_tracing.png";

    for (int i = 1; i < argc; ++i) {
        try {
            if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
                width = std::stoi(argv[++i]);
            } else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
                height = std::stoi(argv[++i]);
            } else if (strcmp(argv[i], "--max-bounces") == 0 && i + 1 < argc) {
                max_bounces = std::stoi(argv[++i]);
            } else if (strcmp(argv[i], "--num-samples") == 0 && i + 1 < argc) {
                num_samples = std::stoi(argv[++i]);
            } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
                output_path = argv[++i];
            } else if (strcmp(argv[i], "--help") == 0) {
                std::cout << "Usage: " << argv[0] << " [--width W] [--height H] [--max-bounces M] [--num-samples N] [--output PATH]\n"
                          << "  --width       Image width in pixels (default: 1280)\n"
                          << "  --height      Image height in pixels (default: 1024)\n"
                          << "  --max-bounces Maximum number of ray bounces (default: 2)\n"
                          << "  --num-samples Number of samples per pixel (default: 100)\n"
                          << "  --output      Output file path (default: ./results/path_tracing.png)\n";
                return 0;
            } else {
                std::cerr << "Unknown or incomplete argument: " << argv[i] << "\n";
                std::cerr << "Use --help for usage information.\n";
                return 1;
            }
        } catch (...) {
            std::cerr << "Invalid value for " << argv[i-1] << ": " << argv[i] << "\n";
            return 1;
        }
    }

    if (width <= 0 || height <= 0 || max_bounces < 0 || num_samples <= 0) {
        std::cerr << "Invalid arguments: width, height, and num_samples must be positive; max-bounces must be non-negative.\n";
        return 1;
    }

    path_tracing(width, height, max_bounces, num_samples, output_path);
    return 0;
}
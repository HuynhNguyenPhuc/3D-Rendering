#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <limits>
#include <cstring>
#include <cstdlib>

#include "vec3.h"
#include "mesh.h"
#include "geometry.h"
#include "primitive_tree.h"
#include "optics.h"
#include "material.h"

const Vec3 BACKGROUND_COLOR(0.1f, 0.1f, 0.1f);

Vec3 cast_ray(const Ray& ray, const Mesh& mesh, const PrimitiveTree& primitives, const std::vector<Light*>& lights) {
    float t;
    Primitive* hit_primitive;

    if (!primitives.intersect(ray, t, hit_primitive)) {
        return BACKGROUND_COLOR;
    }

    Vec3 hit_point = ray.position(t);
    hit_primitive->setHitPoint(hit_point);
    
    Vec3 shading_normal = hit_primitive->getNormal(hit_point);
    Triangle* triangle = dynamic_cast<Triangle*>(hit_primitive);
    Vec3 geometric_normal;

    if (triangle) {
        geometric_normal = triangle->getFaceNormal();
    } else {
        geometric_normal = shading_normal;
    }

    if (shading_normal.dot(ray.direction) > 1e-9) {
        shading_normal = -shading_normal;
    }
    if (geometric_normal.dot(ray.direction) > 1e-9) {
        geometric_normal = -geometric_normal;
    }

    Vec3 base_color;
    if (triangle) {
        Vec3 texture_coordinate = hit_primitive->getTextureCoordinates();
        float u = texture_coordinate[0];
        float v = texture_coordinate[1];
        base_color = mesh.getColorAtUV(u, v);
    } else {
        base_color = hit_primitive->material.color;
    }

    const float ambient_intensity = hit_primitive->material.kA;
    // Ambient term
    Vec3 final_color = base_color * ambient_intensity;


    for (const auto& light : lights) {
        Vec3 light_direction = (light->position - hit_point).normalize();
        float light_distance = (light->position - hit_point).length();

        Ray shadow_ray(hit_point + geometric_normal * 1e-4, light_direction);
        float tShadow;
        Primitive* shadow_hit_primitive;
        
        bool isInShadow = primitives.intersect(shadow_ray, tShadow, shadow_hit_primitive) && tShadow < light_distance;
        
        if (!isInShadow && geometric_normal.dot(light_direction) > 0) {
            // Diffuse term
            float diffuse_intensity = std::max(0.0f, shading_normal.dot(light_direction));
            Vec3 diffuse = base_color * hit_primitive->material.kD * diffuse_intensity * light->intensity;
            
            // Specular term
            Vec3 view_dir = -ray.direction;
            Vec3 halfway_dir = (light_direction + view_dir).normalize();
            float spec_angle = std::max(0.0f, shading_normal.dot(halfway_dir));
            float spec_intensity = std::pow(spec_angle, hit_primitive->material.shininess);
            Vec3 specular = Vec3(1.0f) * hit_primitive->material.kS * spec_intensity * light->intensity;

            // Final color
            final_color += diffuse + specular;
        }
    }

    return final_color;
}

void render(int width, int height, const std::string& output_path, const std::string& mesh_path, const std::string& texture_path, int texture_width, int texture_height) {
    unsigned char* image = new unsigned char[width * height * 3]();

    Mesh mesh;
    std::vector<float> vertex_array;
    if (mesh.load(mesh_path, texture_path, texture_width, texture_height)){
        vertex_array = mesh.getVertexArray();
        std::cout << "Texture loaded successfully!" << std::endl;
    } else {
        std::cerr << "Failed to load " + mesh_path + " or texture!" << std::endl;
        return;
    }

    if (vertex_array.empty()) {
        std::cerr << "Failed to load " + mesh_path + "!" << std::endl;
        return;
    }

    std::vector<Primitive*> primitive_pointers;

    Vec3 primitive_color(1.0f, 0.0f, 0.0f);
    Material material(primitive_color, 0.8f, 0.2f, 0.3f, 16.0f);

    for (size_t i = 0; i < vertex_array.size(); i += 24) {
        Vec3 v0(vertex_array[i], vertex_array[i+1], vertex_array[i+2]);
        Vec3 v1(vertex_array[i+8], vertex_array[i+9], vertex_array[i+10]);
        Vec3 v2(vertex_array[i+16], vertex_array[i+17], vertex_array[i+18]);
        Vec3 n0 = Vec3(vertex_array[i+5], vertex_array[i+6], vertex_array[i+7]).normalize();
        Vec3 n1 = Vec3(vertex_array[i+13], vertex_array[i+14], vertex_array[i+15]).normalize();
        Vec3 n2 = Vec3(vertex_array[i+21], vertex_array[i+22], vertex_array[i+23]).normalize();
        Vec3 st0(vertex_array[i+3], vertex_array[i+4], 0.0f);
        Vec3 st1(vertex_array[i+11], vertex_array[i+12], 0.0f);
        Vec3 st2(vertex_array[i+19], vertex_array[i+20], 0.0f);
        primitive_pointers.push_back(new Triangle(v0, v1, v2, n0, n1, n2, st0, st1, st2, material));
    }

    PrimitiveTree primitives(primitive_pointers);
    
    Vec3 camera(0.0f, 0.5f, 1.0f);
    std::vector<Light*> lights;
    lights.push_back(new Light(Vec3(0.0f, 0.75, 1.5f), Vec3(1.0f, 1.0f, 1.0f), 5.0f));

    float fov = 90.0f * M_PI / 180.0f;
    float aspect = float(width) / float(height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3;

            float px = tan(fov / 2.0f) * (2 * (x + 0.5f) / float(width) - 1) * aspect;
            float py = tan(fov / 2.0f) * (1 - 2 * (y + 0.5f) / float(width));

            Ray ray(camera, Vec3(px, py, -1).normalize());
            Vec3 color = cast_ray(ray, mesh, primitives, lights);

            auto to_srgb = [](float c){ return powf(std::clamp(c, 0.0f, 1.0f), 1.0f/2.2f); };
            image[index]   = (unsigned char)(to_srgb(color.x) * 255.0f);
            image[index + 1] = (unsigned char)(to_srgb(color.y) * 255.0f);
            image[index + 2] = (unsigned char)(to_srgb(color.z) * 255.0f);
        }
    }

    for (Primitive* primitive : primitive_pointers){
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
    std::string output_path = "./results/rendering.png";
    std::string mesh_path = "./models/barrel.obj";
    std::string texture_path = "./models/barrel.png";
    int texture_width = 4096;
    int texture_height = 4096;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0) {
            std::cout << "Usage: " << argv[0] << " [options]\n\n"
                      << "Options:\n"
                      << "  --help                  Show this help message\n"
                      << "  --width <pixels>        Set the output image width (default: 1280)\n"
                      << "  --height <pixels>       Set the output image height (default: 1024)\n"
                      << "  --output <path>         Set the output PNG file path\n"
                      << "  --mesh <path>           Set the path to the .obj mesh file\n"
                      << "  --texture <path>        Set the path to the texture file\n"
                      << "  --tex-width <pixels>    Set the texture width (default: 4096)\n"
                      << "  --tex-height <pixels>   Set the texture height (default: 4096)\n";
            return 0;
        } else if (strcmp(argv[i], "--width") == 0) {
            if (i + 1 < argc) { width = std::atoi(argv[++i]); }
        } else if (strcmp(argv[i], "--height") == 0) {
            if (i + 1 < argc) { height = std::atoi(argv[++i]); }
        } else if (strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) { output_path = argv[++i]; }
        } else if (strcmp(argv[i], "--mesh") == 0) {
            if (i + 1 < argc) { mesh_path = argv[++i]; }
        } else if (strcmp(argv[i], "--texture") == 0) {
            if (i + 1 < argc) { texture_path = argv[++i]; }
        } else if (strcmp(argv[i], "--tex-width") == 0) {
            if (i + 1 < argc) { texture_width = std::atoi(argv[++i]); }
        } else if (strcmp(argv[i], "--tex-height") == 0) {
            if (i + 1 < argc) { texture_height = std::atoi(argv[++i]); }
        } else {
            std::cerr << "Unknown argument: " << argv[i] << std::endl;
            std::cerr << "Use --help for usage information." << std::endl;
            return 1;
        }
    }

    std::cout << "Rendering with the following settings:\n"
              << "  Resolution: " << width << "x" << height << "\n"
              << "  Output: " << output_path << "\n"
              << "  Mesh: " << mesh_path << "\n"
              << "  Texture: " << texture_path << " (" << texture_width << "x" << texture_height << ")\n";

    render(width, height, output_path, mesh_path, texture_path, texture_width, texture_height);

    return 0;
}
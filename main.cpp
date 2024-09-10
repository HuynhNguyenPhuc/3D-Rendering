#include <iostream>
#include <fstream>
#include <cmath>
#include <random>
#include <vector>
#include <cstdlib>

const int WIDTH = 640;
const int HEIGHT = 480;
const int NUM_LIGHTS = 2; 

class Vec3 {
    public:
        float x, y, z;

    public:
        Vec3() : x(0), y(0), z(0) {}
        Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

        Vec3 operator + (const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
        Vec3 operator - () {return Vec3(-x, -y, -z);}
        Vec3 operator - (const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
        Vec3 operator * (float k) const { return Vec3(x * k, y * k, z * k); }
        Vec3 operator / (float k) const { return Vec3(x / k, y / k, z / k); }

        float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }

        Vec3 normalize() const { 
            float mag = std::sqrt(x * x + y * y + z * z); 
            return Vec3(x / mag, y / mag, z / mag); 
        }

        static Vec3 random() {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_real_distribution<> dis(0.0, 1.0);

            float theta = acos(dis(gen));
            float phi = 2.0f * M_PI * dis(gen);

            float x = sin(theta) * cos(phi);
            float y = cos(theta);
            float z = sin(theta) * sin(phi);

            return Vec3(x, y, z).normalize();
        }
};

class Ray {
    public:
        Vec3 origin;
        Vec3 direction;

    public:
        Ray(const Vec3& origin, const Vec3& direction) : origin(origin), direction(direction.normalize()) {}
};

class Sphere {
    public:
        Vec3 center;
        float radius;

    public:
        Sphere(const Vec3& center, float radius) : center(center), radius(radius) {}

        bool intersect(const Ray& ray, float& t) const {
            Vec3 oc = ray.origin - center;
            float a = ray.direction.dot(ray.direction);
            float b = 2.0 * oc.dot(ray.direction);
            float c = oc.dot(oc) - radius * radius;
            float discriminant = b * b - 4 * a * c;
            if (discriminant < 0) return false;
            else {
                t = (-b - std::sqrt(discriminant)) / (2.0 * a);
                return true;
            }
        }
};

void save(const std::string& filename, const unsigned char* data) {
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
    ofs.write(reinterpret_cast<const char*>(data), WIDTH * HEIGHT * 3);
    ofs.close();
}

Vec3 shade(const Vec3& point, const Vec3& normal, const Vec3& light_direction, float light_intensity) {
    Vec3 light_direction_normalized = light_direction.normalize();
    float cos_theta = std::max(0.0f, normal.dot(light_direction_normalized));
    float irradiance = std::max(std::min(light_intensity * cos_theta, 1.0f), 0.0f);
    return Vec3(255.0f, 0.0f, 0.0f) * irradiance;
}

std::vector<Vec3> generate_light_directions(int numLights) {
    std::vector<Vec3> light_directions;
    for (int i = 0; i < numLights; ++i) {
        light_directions.push_back(Vec3::random());
    }
    return light_directions;
}

/* Function to compute irradiance using multiple light directions */
Vec3 hemisphere_shade(const Vec3& point, const Vec3& normal, const std::vector<Vec3>& light_directions, float light_intensity) {
    Vec3 totalIrradiance(0.0f, 0.0f, 0.0f);

    for (const Vec3& light_direction : light_directions) {
        float cos_theta = std::max(0.0f, normal.dot(light_direction));
        float irradiance = light_intensity * cos_theta;
        totalIrradiance = totalIrradiance + Vec3(255.0f, 0.0f, 0.0f) * irradiance;
    }

    return totalIrradiance / float(light_directions.size());
}

void ray_tracing(const std::string& mode) {
    unsigned char image[WIDTH * HEIGHT * 3] = {0};

    Vec3 camera(0, 0, 0);
    Sphere sphere(Vec3(0, 0, -5), 2.0);
    Vec3 light_direction(-5, -5, 5);
    float light_intensity = 1.0f;

    std::string output_path;
    std::vector<Vec3> light_directions = generate_light_directions(NUM_LIGHTS);

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            int index = (y * WIDTH + x) * 3;

            float px = (2 * (x + 0.5f) / float(WIDTH) - 1) * WIDTH / float(HEIGHT);
            float py = (1 - 2 * (y + 0.5f) / float(HEIGHT));

            Ray ray(camera, Vec3(px, py, -1));

            float t;
            if (sphere.intersect(ray, t)) {
                Vec3 hit_point = ray.origin + ray.direction * t;
                Vec3 normal = (hit_point - sphere.center).normalize();

                Vec3 color;
                if (mode == "single") {
                    color = shade(hit_point, normal, light_direction, light_intensity);
                    output_path = "./results/single_light_source_output.ppm";
                } else if (mode == "multiple") {
                    color = hemisphere_shade(hit_point, normal, light_directions, light_intensity);
                    output_path = "./results/multiple_light_sources_output.ppm";
                }

                // Sphere color
                image[index] = static_cast<unsigned char>(std::min(color.x, 255.0f));
                image[index + 1] = static_cast<unsigned char>(std::min(color.y, 255.0f));
                image[index + 2] = static_cast<unsigned char>(std::min(color.z, 255.0f));
            } else {
                // Background color
                image[index] = 0;
                image[index + 1] = 127;
                image[index + 2] = 127;
            }
        }
    }

    save(output_path, image);

    std::cout << "Image saved as " << output_path << std::endl;
}

int main() {
    ray_tracing("single");
    ray_tracing("multiple");

    return 0;
}

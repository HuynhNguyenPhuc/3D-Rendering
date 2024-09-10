#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>

const int WIDTH = 640;
const int HEIGHT = 480;

class Vec3 {
public:
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator + (const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator - (const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator * (float k) const { return Vec3(x * k, y * k, z * k); }
    Vec3 operator * (const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3 operator / (float k) const { return Vec3(x / k, y / k, z / k); }

    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }

    Vec3 normalize() const { 
        float mag = std::sqrt(x * x + y * y + z * z); 
        if (mag > 0) {
            return Vec3(x / mag, y / mag, z / mag); 
        } else {
            return *this;
        }
    }
};

class Ray {
public:
    Vec3 origin;
    Vec3 direction;

    Ray(const Vec3& origin, const Vec3& direction) : origin(origin), direction(direction.normalize()) {}
};

class Sphere {
public:
    Vec3 center;
    float radius;

    Sphere(const Vec3& center, float radius) : center(center), radius(radius) {}

    bool intersect(const Ray& ray, float& t) const {
        Vec3 oc = ray.origin - center;
        float a = ray.direction.dot(ray.direction);
        float b = 2.0f * oc.dot(ray.direction);
        float c = oc.dot(oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        if (discriminant < 0) return false;
        else {
            t = (-b - std::sqrt(discriminant)) / (2.0f * a);
            return true;
        }
    }
};

class Plane {
public:
    Vec3 point;  // Một điểm trên mặt phẳng
    Vec3 normal; // Phương pháp của mặt phẳng

    Plane(const Vec3& point, const Vec3& normal) : point(point), normal(normal.normalize()) {}

    bool intersect(const Ray& ray, float& t) const {
        float denom = normal.dot(ray.direction);
        if (std::fabs(denom) > 1e-6f) {
            Vec3 p0l0 = point - ray.origin;
            t = p0l0.dot(normal) / denom;
            return (t >= 0);
        }
        return false;
    }
};

void save(const std::string& filename, const unsigned char* data) {
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
    ofs.write(reinterpret_cast<const char*>(data), WIDTH * HEIGHT * 3);
    ofs.close();
}

Vec3 shade(const Vec3& hit_point, const Vec3& normal, const Vec3& light_position, const Vec3& light_dir, const Vec3& albedo, const Vec3& sphere_color, float base_intensity) {
    Vec3 l = light_dir.normalize();

    float distance_squared = (hit_point - light_position).dot(hit_point - light_position);
    float attenuation = 1.0f / (distance_squared + 1e-4f);

    float cos_theta = std::max(0.0f, normal.dot(l));

    Vec3 diffuse = albedo * sphere_color * base_intensity * attenuation * cos_theta;
    return diffuse;
}

void ray_tracing() {
    unsigned char image[WIDTH * HEIGHT * 3] = {0};

    Vec3 camera(0, 0, 0);
    Sphere sphere(Vec3(0, 0, -5), 2.0);
    Vec3 light_position(5, 5, 4);
    Vec3 sphere_color(0.5f, 1.0f, 0.5f);
    Vec3 albedo(0.2f, 0.8f, 0.2f);
    float base_intensity = 100.0f;

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
                Vec3 light_dir = (light_position - hit_point).normalize();

                Vec3 color = shade(hit_point, normal, light_position, light_dir, albedo, sphere_color, base_intensity);

                image[index] = static_cast<unsigned char>(std::min(color.x * 255.0f, 255.0f));
                image[index + 1] = static_cast<unsigned char>(std::min(color.y * 255.0f, 255.0f));
                image[index + 2] = static_cast<unsigned char>(std::min(color.z * 255.0f, 255.0f));
            } else {
                image[index] = 127.0;
                image[index + 1] = 127.5;
                image[index + 2] = 127.0;
            }
        }
    }

    save("./results/output.ppm", image);
    std::cout << "Image saved as ./results/output.ppm" << std::endl;
}

int main() {
    ray_tracing();
    return 0;
}
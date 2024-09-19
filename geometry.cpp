#include "geometry.h"
#include "bbox.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>


/*
 * 3D Vector
 */
Vec3::Vec3() : x(0), y(0), z(0) {}

Vec3::Vec3(float value) : x(value), y(value), z(value) {}

Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

float Vec3::operator [] (int idx) const {
    if (idx == 0) return x;
    else if (idx == 1) return y;
    else if (idx == 2) return z;
    else throw std::out_of_range("Index out of range!");
}
float Vec3::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vec3 Vec3::operator + (const Vec3& v) const {
    return Vec3(x + v.x, y + v.y, z + v.z);
}

Vec3 Vec3::operator - (const Vec3& v) const {
    return Vec3(x - v.x, y - v.y, z - v.z);
}

Vec3 Vec3::operator * (float k) const {
    return Vec3(x * k, y * k, z * k);
}

Vec3 Vec3::operator * (const Vec3& v) const {
    return Vec3(x * v.x, y * v.y, z * v.z);
}

Vec3 Vec3::operator / (float k) const {
    return Vec3(x / k, y / k, z / k);
}

float Vec3::dot(const Vec3& v) const {
    return x * v.x + y * v.y + z * v.z;
}

Vec3 Vec3::cross(const Vec3& v) const {
    return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

Vec3 Vec3::normalize() const {
    float length = std::sqrt(x * x + y * y + z * z);
    return Vec3(x / length, y / length, z / length);
}


/*
 * Ray
 */
Ray::Ray(const Vec3& origin, const Vec3& direction)
    : origin(origin), direction(direction.normalize()) {}

/*
 * Light
 */
Light::Light(const Vec3& position, const Vec3& color, float intensity):
    position(position), color(color), intensity(intensity) {}


/*
 * Sphere
 */
Sphere::Sphere(const Vec3& center, float radius, const Vec3& color, float albedo) 
    : center(center), radius(radius), color(color), albedo(albedo) {}

bool Sphere::intersect(const Ray& ray, float& t) const {
    Vec3 oc = ray.origin - center;
    float a = ray.direction.dot(ray.direction);
    float b = 2.0f * oc.dot(ray.direction);
    float c = oc.dot(oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;
    t = (-b - std::sqrt(discriminant)) / (2.0f * a);
    return t >= 0;
}

bool Sphere::intersect(const Ray &ray, float &t_min, float &t_max) const {
    Vec3 oc = ray.origin - center;
    float a = ray.direction.dot(ray.direction);
    float b = 2.0f * oc.dot(ray.direction);
    float c = oc.dot(oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;
    t_min = (-b - std::sqrt(discriminant)) / (2.0f * a);
    t_max = (-b + std::sqrt(discriminant)) / (2.0f * a);
    if (t_max < 0) return false;
    if (t_min < 0) t_min = t_max;
    return true;
}

/*
 * Plane
 */
Plane::Plane(const Vec3& normal, float d, const Vec3& color, float albedo) 
    : normal(normal), d(d), color(color), albedo(albedo) {}

bool Plane::intersect(const Ray& ray, float& t) const {
    float denom = normal.dot(ray.direction);
    if (std::abs(denom) > 1e-6) {
        t = -(normal.dot(ray.origin) + d) / denom;
        return t >= 0;
    }
    return false;
}


/*
 * Triangle
 */
Triangle::Triangle(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& color, float albedo) 
    : p0(p0), p1(p1), p2(p2), color(color), albedo(albedo) {}

bool Triangle::intersect(const Ray& ray, float& t) const {
    Vec3 edge1 = p1 - p0;
    Vec3 edge2 = p2 - p0;
    Vec3 h = ray.direction.cross(edge2);
    float a = edge1.dot(h);
    if (a > -1e-6 && a < 1e-6) return false;
    float f = 1.0f / a;
    Vec3 s = ray.origin - p0;
    float u = f * s.dot(h);
    if (u < 0.0f || u > 1.0f) return false;
    Vec3 q = s.cross(edge1);
    float v = f * ray.direction.dot(q);
    if (v < 0.0f || u + v > 1.0f) return false;
    t = f * edge2.dot(q);
    return t > 1e-6;
}

BoundingBox Triangle::getBoundingBox() const {
    Vec3 minVec(
        std::min(std::min(p0.x, p1.x), p2.x),
        std::min(std::min(p0.y, p1.y), p2.y),
        std::min(std::min(p0.z, p1.z), p2.z)
    );

    Vec3 maxVec(
        std::max(std::max(p0.x, p1.x), p2.x),
        std::max(std::max(p0.y, p1.y), p2.y),
        std::max(std::max(p0.z, p1.z), p2.z)
    );

    return BoundingBox(minVec, maxVec);
}
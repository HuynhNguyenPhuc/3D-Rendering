#include "geometry.h"
#include "vec3.h"
#include "material.h"
#include "bbox.h"
#include <cmath>
#include <algorithm>

/*
 * Ray
 */
Ray::Ray(const Vec3& origin, const Vec3& direction)
    : origin(origin), direction(direction.normalize()) {}

Vec3 Ray::position(float t) const {
    return origin + direction * t;
}

/*
 * Light
 */
Light::Light(const Vec3 &position, const Vec3& color, float intensity){
    this->position = position;
    this->color = color;
    this->intensity = intensity;
}


/*
 * Primitive Base Class
 */
Primitive::Primitive(const Material &material) : material(material) {}


/*
 * Sphere
 */
Sphere::Sphere(const Vec3& center, float radius, const Material &material) 
    : Primitive(material), center(center), radius(radius) {}

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

Vec3 Sphere::getNormal(const Vec3 &point) const {
    return (point - center).normalize();
}

BoundingBox Sphere::getBoundingBox() const {
    Vec3 radiusVec(radius, radius, radius);
    return BoundingBox(center - radiusVec, center + radiusVec);
}


/*
 * Plane
 */
Plane::Plane(const Vec3& normal, float d, const Material &material)
    : Primitive(material), normal(normal), d(d) {}

bool Plane::intersect(const Ray& ray, float& t) const {
    float denom = normal.dot(ray.direction);
    if (std::abs(denom) > 1e-6) {
        t = -(normal.dot(ray.origin) + d) / denom;
        return t >= 0;
    }
    return false;
}

Vec3 Plane::getNormal(const Vec3 &point) const {
    return normal;
}

BoundingBox Plane::getBoundingBox() const {
    Vec3 minPoint(-INFINITY, -INFINITY, -INFINITY);
    Vec3 maxPoint(INFINITY, INFINITY, INFINITY);
    
    return BoundingBox(minPoint, maxPoint);
}


/*
 * Triangle
 */
Triangle::Triangle(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Material &material)
    : Primitive(material), p0(p0), p1(p1), p2(p2) {}

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

Vec3 Triangle::getNormal(const Vec3& point) const {
    Vec3 edge1 = p1 - p0;
    Vec3 edge2 = p2 - p0;
    Vec3 normal = edge1.cross(edge2).normalize();

    return normal;
}

Vec3 Triangle::getNormalFromDirection(const Vec3 &ray_direction) const
{
    Vec3 edge1 = p1 - p0;
    Vec3 edge2 = p2 - p0;
    Vec3 normal = edge1.cross(edge2).normalize();

    float epsilon = 1e-6;
    if (normal.dot(-ray_direction) < epsilon) {
        normal = -normal;
    }

    return normal;
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



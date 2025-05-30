#include "geometry.h"
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
Light::Light(const Vec3 &position, const Vec3& color, float intensity) {
    this->position = position;
    this->color = color;
    this->intensity = intensity;
}

/*
 * Primitive Base Class
 */
Primitive::Primitive(const Material &material) : material(material) {}

void Primitive::setHitPoint(const Vec3 &hit_point) {
    return;
}

Vec3 Primitive::getTextureCoordinates() const{
    return Vec3();
}

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

bool Sphere::intersect(const Ray& ray, float& t0, float& t1) {
    Vec3 oc = ray.origin - center;
    float a = ray.direction.dot(ray.direction);
    float b = 2.0f * oc.dot(ray.direction);
    float c = oc.dot(oc) - radius * radius;
    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f) return false;

    float sqrtDiscriminant = std::sqrt(discriminant);
    float inv2a = 0.5f / a;

    t0 = (-b - sqrtDiscriminant) * inv2a;
    t1 = (-b + sqrtDiscriminant) * inv2a;

    if (t0 > t1) std::swap(t0, t1);

    return true;
}

Vec3 Sphere::getNormal(const Vec3 &hit_point) const {
    return (hit_point - center).normalize();
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

Vec3 Plane::getNormal(const Vec3 &hit_point) const {
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
void Triangle::setHitPoint(const Vec3& hit_point) {
    Vec3 v0 = p1 - p0;
    Vec3 v1 = p2 - p0;
    Vec3 v2 = hit_point - p0;

    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d11 = v1.dot(v1);
    float d20 = v2.dot(v0);
    float d21 = v2.dot(v1);

    float denom = d00 * d11 - d01 * d01;
    if (std::abs(denom) < 1e-8f) {
        u = v = 0.0f;
        return;
    }

    float invDenom = 1.0f / denom;
    u = (d11 * d20 - d01 * d21) * invDenom;
    v = (d00 * d21 - d01 * d20) * invDenom;
}

// Möller–Trumbore algorithm for intersection points
bool Triangle::intersect(const Ray& ray, float& t) const {
    Vec3 edge1 = p1 - p0;
    Vec3 edge2 = p2 - p0;
    Vec3 h = ray.direction.cross(edge2);
    float a = edge1.dot(h);
    if (a > -1e-6 && a < 1e-6) return false;

    float f = 1.0f / a;
    Vec3 s = ray.origin - p0;
    u = f * s.dot(h);
    if (u < 0.0f || u > 1.0f) return false;

    Vec3 q = s.cross(edge1);
    v = f * ray.direction.dot(q);
    if (v < 0.0f || u + v > 1.0f) return false;

    t = f * edge2.dot(q);
    return t > 1e-6;
}

Vec3 Triangle::getNormal(const Vec3& hit_point) const {
    return (n1 * (1 - u - v) + n2 * u + n3 * v).normalize();
}

Vec3 Triangle::getTextureCoordinates() const {
    Vec3 result = st1 * (1 - u - v) + st2 * u + st3 * v;
    return wrap_around(result);
}

Vec3 Triangle::getNormalFromDirection(const Vec3& ray_direction) const {
    Vec3 normal = (p1 - p0).cross(p2 - p0).normalize();
    if (normal.dot(-ray_direction) < 1e-6) {
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
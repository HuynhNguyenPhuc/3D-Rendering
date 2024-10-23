#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "vec3.h"
#include "material.h" 

class BoundingBox;

class Ray {
public:
    Vec3 origin;
    Vec3 direction;
    Ray(const Vec3& origin, const Vec3& direction);
    Vec3 position(float t) const;
};

class Light{
public:
    Vec3 position;
    Vec3 color;
    float intensity;

    Light(const Vec3& position, const Vec3& color, float intensity);
};

class Primitive {
public:
    Material material;
    Primitive(const Material &material);
    virtual ~Primitive() = default;
    virtual bool intersect(const Ray& ray, float& t) const = 0;
    virtual Vec3 getNormal(const Vec3& point) const = 0;
    virtual BoundingBox getBoundingBox() const = 0;
};

class Sphere : public Primitive {
public:
    Vec3 center;
    float radius;

    Sphere(const Vec3& center, float radius, const Material &material);
    bool intersect(const Ray& ray, float& t) const override;
    Vec3 getNormal(const Vec3& point) const override;
    BoundingBox getBoundingBox() const override;
};

class Plane : public Primitive {
public:
    Vec3 normal;
    float d;

    Plane(const Vec3& normal, float d, const Material &material);
    bool intersect(const Ray& ray, float& t) const override;
    Vec3 getNormal(const Vec3& point) const override;
    BoundingBox getBoundingBox() const override;
};

class Triangle : public Primitive {
public:
    Vec3 p0, p1, p2;

    Triangle(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Material &material);
    bool intersect(const Ray& ray, float& t) const override;
    Vec3 getNormal(const Vec3& point) const override;
    Vec3 getNormalFromDirection(const Vec3& ray_direction) const;
    BoundingBox getBoundingBox() const override;
};

#endif // GEOMETRY_H
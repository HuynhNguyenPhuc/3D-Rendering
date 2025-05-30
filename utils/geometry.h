#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "vec3.h"
#include "material.h" 
#include "utils.h"

class BoundingBox;

class Ray {
public:
    Vec3 origin;
    Vec3 direction;
    Ray(const Vec3& origin, const Vec3& direction);
    Vec3 position(float t) const;
};

class Light {
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
    virtual void setHitPoint(const Vec3& hit_point);
    virtual bool intersect(const Ray& ray, float& t) const = 0;
    virtual Vec3 getNormal(const Vec3& hit_point) const = 0;
    virtual Vec3 getTextureCoordinates() const;
    virtual BoundingBox getBoundingBox() const = 0;
};

class Sphere : public Primitive {
public:
    Vec3 center;
    float radius;

    Sphere(const Vec3& center, float radius, const Material &material);
    bool intersect(const Ray& ray, float& t) const override;
    bool intersect(const Ray& ray, float& t0, float& t1);
    Vec3 getNormal(const Vec3& hit_point) const override;
    BoundingBox getBoundingBox() const override;
};

class Plane : public Primitive {
public:
    Vec3 normal;
    float d;

    Plane(const Vec3& normal, float d, const Material &material);
    bool intersect(const Ray& ray, float& t) const override;
    Vec3 getNormal(const Vec3& hit_point) const override;
    BoundingBox getBoundingBox() const override;
};

class Triangle : public Primitive {
public:
    Vec3 p0, p1, p2;         // Vertex positions
    Vec3 n1, n2, n3;         // Vertex normals
    Vec3 st1, st2, st3;      // Texture coordinates

private:
    mutable float u, v;              // Barycentric coordinates

public:
    Triangle(
        const Vec3& p0, const Vec3& p1, const Vec3& p2, 
        const Vec3& n1, const Vec3& n2, const Vec3& n3, 
        const Vec3& st1, const Vec3& st2, const Vec3& st3, 
        const Material &material
    ): 
        Primitive(material),
        p0(p0), p1(p1), p2(p2), 
        n1(n1.normalize()), n2(n2.normalize()), n3(n3.normalize()), 
        st1(st1), st2(st2), st3(st3),
        u(0), v(0) 
    {}

    void setHitPoint(const Vec3& hit_point);
    bool intersect(const Ray& ray, float& t) const override;
    Vec3 getNormal(const Vec3& hit_point) const override;
    Vec3 getTextureCoordinates() const;
    Vec3 getNormalFromDirection(const Vec3& ray_direction) const;
    BoundingBox getBoundingBox() const override;
};

#endif // GEOMETRY_H
#ifndef GEOMETRY_H
#define GEOMETRY_H

class BoundingBox;

class Vec3 {
public:
    float x, y, z;
    Vec3();
    Vec3(float value);
    Vec3(float x, float y, float z);
    float operator [] (int idx) const;
    float length() const;
    Vec3 operator + (const Vec3& v) const;
    Vec3 operator - (const Vec3& v) const;
    Vec3 operator * (float k) const;
    Vec3 operator * (const Vec3& v) const;
    Vec3 operator / (float k) const;
    float dot(const Vec3& v) const;
    Vec3 cross(const Vec3& v) const;
    Vec3 normalize() const;
};

class Ray {
public:
    Vec3 origin;
    Vec3 direction;
    Ray(const Vec3& origin, const Vec3& direction);
};

class Light {
public:
    Vec3 position;
    Vec3 color;
    float intensity;

    Light(const Vec3& position, const Vec3& color, float intensity);
};

class Sphere {
public:
    Vec3 center;
    float radius;
    Vec3 color;
    float albedo;

    Sphere(const Vec3& center, float radius, const Vec3& color, float albedo);
    bool intersect(const Ray& ray, float& t) const;
    bool intersect(const Ray& ray, float& t_min, float& t_max) const;
};

class Plane {
public:
    Vec3 normal;
    float d;
    Vec3 color;
    float albedo;

    Plane(const Vec3& normal, float d, const Vec3& color, float albedo);
    bool intersect(const Ray& ray, float& t) const;
};

class Triangle {
public:
    Vec3 p0, p1, p2;
    Vec3 color;
    float albedo;

    Triangle(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& color, float albedo);
    bool intersect(const Ray& ray, float& t) const;
    BoundingBox getBoundingBox() const;
};

#endif // GEOMETRY_H
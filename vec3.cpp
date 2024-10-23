#include "vec3.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>

/*
 * 3D Vector
 */
Vec3::Vec3() : x(0), y(0), z(0) {}

Vec3::Vec3(float value) : x(value), y(value), z(value) {}

Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

float Vec3::operator[](int idx) const {
    if (idx == 0) return x;
    else if (idx == 1) return y;
    else if (idx == 2) return z;
    else throw std::out_of_range("Index out of range!");
}

float& Vec3::operator[](int idx) {
    if (idx == 0) return x;
    else if (idx == 1) return y;
    else if (idx == 2) return z;
    else throw std::out_of_range("Index out of range!");
}

float Vec3::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

bool Vec3::operator ==(const Vec3 &v) const {
    return (std::abs(x - v.x) < 1e-6) && (std::abs(y - v.y) < 1e-6) && (std::abs(z - v.z) < 1e-6);
}

bool Vec3::operator!=(const Vec3 &v) const {
    return !operator==(v);
}

Vec3 Vec3::operator+(const Vec3& v) const {
    return Vec3(x + v.x, y + v.y, z + v.z);
}

void Vec3::operator+=(const Vec3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
}

Vec3 Vec3::operator-() const {
    return Vec3(-x, -y, -z);
}

Vec3 Vec3::operator-(const Vec3& v) const {
    return Vec3(x - v.x, y - v.y, z - v.z);
}

Vec3 Vec3::operator*(float k) const {
    return Vec3(x * k, y * k, z * k);
}

Vec3 Vec3::operator*(const Vec3& v) const {
    return Vec3(x * v.x, y * v.y, z * v.z);
}

Vec3 Vec3::operator/(float k) const {
    return Vec3(x / k, y / k, z / k);
}

float Vec3::dot(const Vec3& v) const {
    return x * v.x + y * v.y + z * v.z;
}

Vec3 Vec3::cross(const Vec3& v) const {
    return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

Vec3 Vec3::normalize() const {
    float len = length();
    return Vec3(x / len, y / len, z / len);
}
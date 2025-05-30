/*
 * Refrence from "Chapter 9.3: Specular Reflection and Transmission" 
 * More details here: https://pbr-book.org/4ed/Reflection_Models/Specular_Reflection_and_Transmission
 */


#include "optics.h"
#include "vec3.h"
#include <algorithm>
#include <cmath>

Vec3 reflection(const Vec3& incident, const Vec3& normal) {
    Vec3 I = incident.normalize();
    Vec3 N = normal.normalize();
    return (I - N * 2.0f * I.dot(N)).normalize();
}

Vec3 refraction(const Vec3& incident, const Vec3& normal, float ior, bool& isInside) {
    Vec3 I = incident.normalize();
    Vec3 N = normal.normalize();

    float cos_i = -I.dot(N);
    float eta = ior;
    isInside = false;

    // Case: The ray is inside the object
    if (cos_i < 0.0f) {
        N = -N;
        cos_i = -cos_i;
        eta = 1.0f / ior;
        isInside = true;
    }

    float sin2_i = 1.0f - cos_i * cos_i;
    float sin2_t = eta * eta * sin2_i; // Snell's law: eta = n_t / n_i = sin_i / sin_t

    // Case: Total internal reflection
    if (sin2_t - 1.0f > 1e-6) {
        return Vec3(0.0f);
    }

    float cos_t = std::sqrt(std::max(1.0f - sin2_t, 0.0f));
    return I / eta + N * (cos_i / eta - cos_t);
}

float fresnel(const Vec3& incident, const Vec3& normal, float ior) {
    Vec3 I = incident.normalize();
    Vec3 N = normal.normalize();

    float cos_i = std::min(std::max(-I.dot(N), -1.0f), 1.0f);
    float eta = ior;

    if (cos_i < 0.0f) {
        eta = 1.0f / ior;
        cos_i = -cos_i;
    }

    // Snell's law
    float sin2_i = 1.0f - cos_i * cos_i;
    float sin2_t = eta * eta * sin2_i;

    if (sin2_t > 1.0f) {
        return 1.0f;
    }

    float cos_t = std::sqrt(std::max(1.0f - sin2_t, 0.0f));
    
    float rs = (eta * cos_i - cos_t) / (eta * cos_i + cos_t);
    float rp = (cos_i - eta * cos_t) / (cos_i + eta * cos_t);
    
    return (rs * rs + rp * rp) / 2.0f;
}
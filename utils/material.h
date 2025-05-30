#ifndef MATERIAL_H
#define MATERIAL_H

#include "vec3.h"

enum MaterialType{
    REFRACTIVE,
    REFLECTIVE,
    NONE
};

class Material {
public:
    Vec3 color;       // Base color of the material
    Vec3 albedo;     // Albedo factor
    float kA;         // Ambient coefficient
    float kD;         // Diffuse coefficient
    float kS;         // Specular coefficient
    float kT;         // Transparency coefficient
    float ior;        // Index of refraction
    float shininess;  // Shininess for specular reflection
    MaterialType type;  // Type of material (refraction, reflection, or none)

    Material(const Vec3& color, const Vec3& albedo, float kA, float kD, float kS, float kT, float ior, float shininess, MaterialType type);
    Material(const Vec3& color);
    Material(const Vec3& color, float kA, float kD, float kS, float shininess);
    Material(const Vec3& color, float kA, float kD, float kS, float ior, float shininess, MaterialType type);
};

#endif // MATERIAL_H
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
    float albedo;     // Albedo factor
    float kA;         // Ambient coefficient
    float kD;         // Diffuse coefficient
    float kS;         // Specular coefficient
    float kT;         // Transparency coefficient
    float ior;        // Index of refraction
    float shininess;  // Shininess for specular reflection
    MaterialType type;  // Type of material (refraction, reflection, or none)

    Material(const Vec3& color, float albedo, float kA, float kD, float kS, float kT, float ior, float shininess, MaterialType type);
};

#endif // MATERIAL_H
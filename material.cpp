#include "material.h"
#include "vec3.h"


Material::Material(const Vec3 &color, float albedo, float kA, float kD, float kS, float kT, float ior, float shininess, MaterialType type){
    this->color = color;
    this->albedo = albedo;
    this->kA = kA;
    this->kD = kD;
    this->kS = kS;
    this->kT = kT;
    this->ior = ior;
    this->shininess = shininess;
    this->type = type;
}
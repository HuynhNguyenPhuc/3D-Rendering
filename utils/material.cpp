#include "material.h"
#include "vec3.h"


Material::Material(const Vec3 &color, const Vec3& albedo, float kA, float kD, float kS, float kT, float ior, float shininess, MaterialType type){
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

Material::Material(const Vec3 &color){
    this->color = color;
    this->albedo = Vec3(1.0f);
    this->kA = 1.0f;
    this->kD = 1.0f;
    this->kS = 1.0f;
    this->kT = 0.0f;
    this->ior = 1.0f;
    this->shininess = 32.0f;
    this->type = MaterialType::NONE;
}

Material::Material(const Vec3 &color, float kA, float kD, float kS, float shininess){
    this->color = color;
    this->albedo = Vec3(1.0f);
    this->kA = kA;
    this->kD = kD;
    this->kS = kS;
    this->kT = 0.0f;
    this->ior = 1.0f;
    this->shininess = shininess;
    this->type = MaterialType::NONE;
}

Material::Material(const Vec3 &color, float kA, float kD, float kS, float ior, float shininess, MaterialType type){
    this->color = color;
    this->albedo = Vec3(1.0f);
    this->kA = kA;
    this->kD = kD;
    this->kS = kS;
    this->kT = 0.0f;
    this->ior = ior;
    this->shininess = shininess;
    this->type = type;
}

#ifndef OPTICS_H
#define OPTICS_H

#include "vec3.h"

Vec3 reflection(const Vec3& incident, const Vec3& normal);
Vec3 refraction(const Vec3& incident, const Vec3& normal, float ior);
float fresnel(const Vec3& incident, const Vec3& normal, float ior);

#endif // OPTICS
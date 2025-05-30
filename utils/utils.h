#ifndef UTILS_H
#define UTILS_H

#include "vec3.h"
#include <string>
#include <cmath>

Vec3 wrap_around(const Vec3& v);
void save_png(const std::string& filename, const unsigned char* data, int width, int height);

#endif // UTILS_H
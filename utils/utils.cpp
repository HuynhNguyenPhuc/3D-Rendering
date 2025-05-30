#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "utils.h"

Vec3 wrap_around(const Vec3& v) {
    float x = v.x - std::floor(v.x);
    float y = v.y - std::floor(v.y);
    float z = v.z - std::floor(v.z);
    return Vec3(x, y, z);
}

void save_png(const std::string& filename, const unsigned char* data, int width, int height) {
    stbi_write_png(filename.c_str(), width, height, 3, data, width * 3);
}
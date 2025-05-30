#ifndef MESH_H
#define MESH_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include "vec3.h"

class Mesh {
public:
    bool load(const std::string& meshFile, const std::string& textureFile, int width, int height);

    std::vector<float> getVertexArray();

    Vec3 getColorAtUV(float u, float v) const;

private:
    std::vector<std::array<float, 3>> vertices;
    std::vector<std::array<float, 3>> normals;
    std::vector<std::array<float, 2>> textures;
    std::vector<std::vector<std::array<int, 3>>> faces;

    std::vector<std::vector<std::array<uint8_t, 3>>> texture;

    int texture_width = 0;
    int texture_height = 0;

    void processVertex(std::istringstream& iss);
    void processTexture(std::istringstream& iss);
    void processNormal(std::istringstream& iss);
    void processFace(std::istringstream& iss);

    bool loadTexture(const std::string& filename, int width, int height);
};

#endif // MESH_H
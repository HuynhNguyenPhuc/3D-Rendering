#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "mesh.h"
#include <cmath>

bool Mesh::load(const std::string& meshFile, const std::string& textureFile, int width, int height) {
    std::ifstream file(meshFile);
    if (!file.is_open()) {
        std::cerr << "Could not open mesh file: " << meshFile << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            processVertex(iss);
        } else if (type == "vt") {
            processTexture(iss);
        } else if (type == "vn") {
            processNormal(iss);
        } else if (type == "f") {
            processFace(iss);
        }
    }
    file.close();

    if (!loadTexture(textureFile, width, height)) {
        return false;
    }

    return true;
}

bool Mesh::loadTexture(const std::string& filename, int width, int height) {
    int texWidth, texHeight, channels;
    unsigned char* data = stbi_load(filename.c_str(), &texWidth, &texHeight, &channels, 3);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return false;
    }

    texture_width = texWidth;
    texture_height = texHeight;
    texture.resize(texture_height, std::vector<std::array<uint8_t, 3>>(texture_width));

    for (int y = 0; y < texture_height; ++y) {
        for (int x = 0; x < texture_width; ++x) {
            int idx = (y * texture_width + x) * 3;
            texture[y][x] = {data[idx], data[idx + 1], data[idx + 2]};
        }
    }

    stbi_image_free(data);
    return true;
}

Vec3 Mesh::getColorAtUV(float u, float v) const {
    float x = u * (texture_width - 1);
    float y = (1.0f - v) * (texture_height - 1);
    
    int x0 = std::max(0, std::min(static_cast<int>(std::floor(x)), texture_width - 1));
    int x1 = std::min(x0 + 1, texture_width - 1);
    int y0 = std::max(0, std::min(static_cast<int>(std::floor(y)), texture_height - 1));
    int y1 = std::min(y0 + 1, texture_height - 1);


    float xWeight = x - x0;
    float yWeight = y - y0;
    
    std::array<uint8_t, 3> c00 = texture[y0][x0];
    std::array<uint8_t, 3> c10 = texture[y0][x1];
    std::array<uint8_t, 3> c01 = texture[y1][x0];
    std::array<uint8_t, 3> c11 = texture[y1][x1];
    
    Vec3 colorTop = Vec3(
        c00[0] * (1 - xWeight) + c10[0] * xWeight,
        c00[1] * (1 - xWeight) + c10[1] * xWeight,
        c00[2] * (1 - xWeight) + c10[2] * xWeight
    ) / 255.0f;

    Vec3 colorBottom = Vec3(
        c01[0] * (1 - xWeight) + c11[0] * xWeight,
        c01[1] * (1 - xWeight) + c11[1] * xWeight,
        c01[2] * (1 - xWeight) + c11[2] * xWeight
    ) / 255.0f;

    Vec3 finalColor = colorTop * (1 - yWeight) + colorBottom * yWeight;

    return finalColor;
}

void Mesh::processVertex(std::istringstream& iss) {
    std::array<float, 3> vertex;
    iss >> vertex[0] >> vertex[1] >> vertex[2];
    vertices.push_back(vertex);
}

void Mesh::processTexture(std::istringstream& iss) {
    std::array<float, 2> texture;
    iss >> texture[0] >> texture[1];
    textures.push_back(texture);
}

void Mesh::processNormal(std::istringstream& iss) {
    std::array<float, 3> normal;
    iss >> normal[0] >> normal[1] >> normal[2];
    normals.push_back(normal);
}

void Mesh::processFace(std::istringstream& iss) {
    std::vector<std::array<int, 3>> face;
    std::string part;
    while (iss >> part) {
        std::array<int, 3> indices = {-1, -1, -1};

        size_t firstSlash = part.find('/');
        size_t secondSlash = part.find('/', firstSlash + 1);

        if (firstSlash != std::string::npos) {
            indices[0] = std::stoi(part.substr(0, firstSlash)) - 1;
            if (secondSlash != std::string::npos) {
                if (firstSlash + 1 != secondSlash) {
                    indices[1] = std::stoi(part.substr(firstSlash + 1, secondSlash - firstSlash - 1)) - 1;
                }
                indices[2] = std::stoi(part.substr(secondSlash + 1)) - 1;
            }
        } else {
            indices[0] = std::stoi(part) - 1;
        }

        face.push_back(indices);
    }
    faces.push_back(face);
}

std::vector<float> Mesh::getVertexArray() {
    std::vector<float> vertexArray;

    for (const auto& face : faces) {
        for (size_t i = 1; i < face.size() - 1; ++i) {
            for (int j = 0; j < 3; ++j) {
                int di = (j == 0) ? -i : j - 1;
                
                const auto& vertex = vertices[face[i + di][0]];
                vertexArray.insert(vertexArray.end(), vertex.begin(), vertex.end());

                if (face[i + di][1] != -1) {
                    const auto& texture = textures[face[i + di][1]];
                    vertexArray.insert(vertexArray.end(), texture.begin(), texture.end());
                } else {
                    vertexArray.insert(vertexArray.end(), {0.0f, 0.0f});
                }

                const auto& normal = normals[face[i + di][2]];
                vertexArray.insert(vertexArray.end(), normal.begin(), normal.end());
            }
        }
    }
    return vertexArray;
}
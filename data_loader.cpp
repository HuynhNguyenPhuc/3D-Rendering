#include "data_loader.h"

std::vector<float> DataLoader::load(const std::string& filename) {
    std::vector<float> vertexArray;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return vertexArray;
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

    vertexArray = getVertexArray();

    vertices.clear();
    normals.clear();
    textures.clear();
    faces.clear();

    file.close();
    return vertexArray;
}

void DataLoader::processVertex(std::istringstream& iss) {
    std::array<float, 3> vertex;
    iss >> vertex[0] >> vertex[1] >> vertex[2];
    vertices.push_back(vertex);
}

void DataLoader::processTexture(std::istringstream& iss) {
    std::array<float, 2> texture;
    iss >> texture[0] >> texture[1];
    textures.push_back(texture);
}

void DataLoader::processNormal(std::istringstream& iss) {
    std::array<float, 3> normal;
    iss >> normal[0] >> normal[1] >> normal[2];
    normals.push_back(normal);
}

void DataLoader::processFace(std::istringstream& iss) {
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

std::vector<float> DataLoader::getVertexArray() {
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
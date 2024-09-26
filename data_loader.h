#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>

class DataLoader {
    public:
        std::vector<float> load(const std::string& filename);

    private:
        std::vector<std::array<float, 3>> vertices;
        std::vector<std::array<float, 3>> normals;
        std::vector<std::array<float, 2>> textures;
        std::vector<std::vector<std::array<int, 3>>> faces;

        void processVertex(std::istringstream& iss);
        void processTexture(std::istringstream& iss);
        void processNormal(std::istringstream& iss);
        void processFace(std::istringstream& iss);

        std::vector<float> getVertexArray();
};

#endif // DATA_LOADER_H
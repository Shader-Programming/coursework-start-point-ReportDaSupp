#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include "Shaders/Shader.h"

class BiomeDistributor {
public:
    BiomeDistributor(const char* computeShaderPath);
    ~BiomeDistributor();

    void generateMaps();
    GLuint getHeightMap() const { return heightMap; }
    GLuint getTemperatureMap() const { return temperatureMap; }
    GLuint getPrecipitationMap() const { return precipitationMap; }

private:
    std::shared_ptr<Shader> computeShader;
    GLuint heightMap, temperatureMap, precipitationMap;
    void initializeMaps();
};

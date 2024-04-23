#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <iostream>

#include "Shaders/Shader.h"

class TerrainGenerator {
public:
    TerrainGenerator(const char* computeShaderPath);
    ~TerrainGenerator();

    void generateTerrainMesh();
    std::vector<GLuint> generateIndices();
    GLuint getTerrainVAO() const { return terrainVAO; }
    int getIndexCount() { return indexCount; };

private:
    std::shared_ptr<Shader> computeShader;
    GLuint terrainVAO, terrainVBO, terrainIBO;
    int indexCount;
    int subdivisions = 16;
    void initializeMesh();
};
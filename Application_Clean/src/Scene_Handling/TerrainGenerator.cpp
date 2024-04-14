#include "Scene_Handling/TerrainGenerator.h"
#include <cmath> 

TerrainGenerator::TerrainGenerator(const char* computeShaderPath) {
    computeShader = std::make_shared<Shader>(computeShaderPath);
    initializeMesh();
    generateTerrainMesh();
}

TerrainGenerator::~TerrainGenerator() {
    glDeleteVertexArrays(1, &terrainVAO);
    glDeleteBuffers(1, &terrainVBO);
    glDeleteBuffers(1, &terrainIBO);

}

void TerrainGenerator::initializeMesh() {
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    int verticesPerSide = (subdivisions + 1) * (subdivisions + 1);  // vertices per face
    int totalVertices = 6 * verticesPerSide;  // 6 faces on a cube

    // Vertex buffer setup
    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * totalVertices, nullptr, GL_STATIC_DRAW);

    // Index buffer setup for subdivided cube (optional, if using indexed drawing)
    glGenBuffers(1, &terrainIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIBO);
    std::vector<GLuint> indices = generateIndices();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

std::vector<GLuint> TerrainGenerator::generateIndices() {
    std::vector<GLuint> indices;
    for (int face = 0; face < 6; ++face) {
        for (int i = 0; i < subdivisions; ++i) {
            for (int j = 0; j < subdivisions; ++j) {
                int start = face * (subdivisions + 1) * (subdivisions + 1) + i * (subdivisions + 1) + j;
                indices.push_back(start);
                indices.push_back(start + 1);
                indices.push_back(start + subdivisions + 1);

                indices.push_back(start + 1);
                indices.push_back(start + subdivisions + 2);
                indices.push_back(start + subdivisions + 1);
            }
        }
    }
    indexCount = indices.size();
    return indices;
}

void TerrainGenerator::generateTerrainMesh() {
    computeShader->use();
    computeShader->setInt("subdivisions", subdivisions);

    glBindVertexArray(terrainVAO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, terrainVBO);

    glDispatchCompute(8,8,1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glBindVertexArray(0);
}
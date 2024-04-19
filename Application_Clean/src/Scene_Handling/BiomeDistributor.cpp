#include "Scene_Handling/BiomeDistributor.h"

BiomeDistributor::BiomeDistributor(const char* computeShaderPath) {
    computeShader = std::make_shared<Shader>(computeShaderPath);
    initializeMaps();
    generateMaps();
}

BiomeDistributor::~BiomeDistributor() {
    glDeleteTextures(1, &heightMap);
    glDeleteTextures(1, &temperatureMap);
    glDeleteTextures(1, &precipitationMap);
}

void BiomeDistributor::initializeMaps() {
    glGenTextures(1, &heightMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, heightMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R32F, 4096, 4096, 0, GL_RED, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);

    glGenTextures(1, &temperatureMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, temperatureMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R32F, 4096, 4096, 0, GL_RED, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);

    glGenTextures(1, &precipitationMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, precipitationMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R32F, 4096, 4096, 0, GL_RED, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void BiomeDistributor::generateMaps() {
    computeShader->use();

    glBindImageTexture(0, heightMap, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
    glBindImageTexture(1, temperatureMap, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
    glBindImageTexture(2, precipitationMap, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

    auto now = std::chrono::high_resolution_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

    std::mt19937 gen(nanos);

    std::uniform_real_distribution<float> dist(0.0, 1.0);
    seed = dist(gen);

    computeShader->setFloat("heightSeed", seed);

    glDispatchCompute(4096 / 8, 4096 / 8, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
    glBindImageTexture(1, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
    glBindImageTexture(2, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
}

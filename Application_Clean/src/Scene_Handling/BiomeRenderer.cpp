#include "Scene_Handling/BiomeRenderer.h"
#include <iostream>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

BiomeRenderer::BiomeRenderer(GLuint vao, const char* shaderPath)
    : planetVAO(vao) {
    lightingShader = std::make_shared<Shader>(shaderPath);
    GLuint sandTexture =    loadTexture("../Resources/Textures/Biomes/SandTexture.jpg");
    GLuint snowTexture =    loadTexture("../Resources/Textures/Biomes/SandTexture.jpg");
    GLuint forestTexture =  loadTexture("../Resources/Textures/Biomes/ForestTexture.jpg");
    GLuint grassTexture =   loadTexture("../Resources/Textures/Biomes/SandTexture.jpg");
    GLuint beachTexture =   loadTexture("../Resources/Textures/Biomes/SandTexture.jpg");
}

BiomeRenderer::~BiomeRenderer() {
    glDeleteVertexArrays(1, &planetVAO);
}

void BiomeRenderer::renderPlanet(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, int indexCount, glm::vec3 cameraPos, float dt) {
    
    timeElapsed += dt;

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    lightingShader->use();
    lightingShader->setMat4("view", viewMatrix);
    lightingShader->setMat4("projection", projectionMatrix);
    glm::mat4 matrix(1.0f);
    lightingShader->setMat4("model", glm::mat4(1.0f));
    lightingShader->setVec3("cameraPos", cameraPos);
    lightingShader->setBool("water", false);
    lightingShader->setFloat("elapsedTime", timeElapsed);

    glBindVertexArray(planetVAO);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glDrawElements(GL_PATCHES, indexCount, GL_UNSIGNED_INT, 0);

    lightingShader->setBool("water", true);
    
    glEnable(GL_BLEND);
    
    glDepthMask(GL_FALSE);

    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glDrawElements(GL_PATCHES, indexCount, GL_UNSIGNED_INT, 0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glBindVertexArray(0);
}

void BiomeRenderer::setupShaderWithMaps(GLuint heightMap, GLuint tempMap, GLuint precipMap) {
    lightingShader->use();

    // Bind and set textures to shader
    lightingShader->setInt("heightMap", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, heightMap);

    lightingShader->setInt("tempMap", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tempMap);

    lightingShader->setInt("precipMap", 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, precipMap);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sandTexture);
    lightingShader->setInt("sandTexture", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, snowTexture);
    lightingShader->setInt("snowTexture", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, forestTexture);
    lightingShader->setInt("forestTexture", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    lightingShader->setInt("grassTexture", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, beachTexture);
    lightingShader->setInt("beachTexture", 4);
}
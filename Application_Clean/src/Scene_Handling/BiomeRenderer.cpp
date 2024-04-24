#include "Scene_Handling/BiomeRenderer.h"
#include <iostream>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "Globals/Properties.h"

BiomeRenderer::BiomeRenderer(GLuint vao, const char* tShaderPath, const char* tPBRShaderPath, const char* wShaderPath, const char* aShaderPath, const char* mShaderPath, int iCount)
    : planetVAO(vao) {
    terrainLightingShader = std::make_shared<Shader>(tShaderPath);
    terrainPBRLightingShader = std::make_shared<Shader>(tPBRShaderPath);
    waterLightingShader = std::make_shared<Shader>(wShaderPath);
    atmosphereLightingShader = std::make_shared<Shader>(aShaderPath);
    moonLightingShader = std::make_shared<Shader>(mShaderPath);

    AlbedoPaths.push_back("../Resources/Textures/Biomes/Dirt/Albedo.jpg");
    AlbedoPaths.push_back("../Resources/Textures/Biomes/Forest/Albedo.jpg");
    AlbedoPaths.push_back("../Resources/Textures/Biomes/Grass/Albedo.jpg");
    AlbedoPaths.push_back("../Resources/Textures/Biomes/Rock/Albedo.jpg");
    AlbedoPaths.push_back("../Resources/Textures/Biomes/Sand/Albedo.jpg");
    AlbedoPaths.push_back("../Resources/Textures/Biomes/Snow/Albedo.jpg");

    Albedo = loadTextureArray(AlbedoPaths);

    NormalPaths.push_back("../Resources/Textures/Biomes/Dirt/Normal.jpg");
    NormalPaths.push_back("../Resources/Textures/Biomes/Forest/Normal.jpg");
    NormalPaths.push_back("../Resources/Textures/Biomes/Grass/Normal.jpg");
    NormalPaths.push_back("../Resources/Textures/Biomes/Rock/Normal.jpg");
    NormalPaths.push_back("../Resources/Textures/Biomes/Sand/Normal.jpg");
    NormalPaths.push_back("../Resources/Textures/Biomes/Snow/Normal.jpg");

    Normal = loadTextureArray(NormalPaths);
    
    indexCount = iCount;

}

void BiomeRenderer::loadTexturesPBR()
{

    AOPaths.push_back("../Resources/Textures/Biomes/Dirt/AO.jpg");
    AOPaths.push_back("../Resources/Textures/Biomes/Forest/AO.jpg");
    AOPaths.push_back("../Resources/Textures/Biomes/Grass/AO.jpg");
    AOPaths.push_back("../Resources/Textures/Biomes/Rock/AO.jpg");
    AOPaths.push_back("../Resources/Textures/Biomes/Sand/AO.jpg");
    AOPaths.push_back("../Resources/Textures/Biomes/Snow/AO.jpg");

    AO = loadTextureArray(AOPaths);

    DisplacementPaths.push_back("../Resources/Textures/Biomes/Dirt/Displacement.jpg");
    DisplacementPaths.push_back("../Resources/Textures/Biomes/Forest/Displacement.jpg");
    DisplacementPaths.push_back("../Resources/Textures/Biomes/Grass/Displacement.jpg");
    DisplacementPaths.push_back("../Resources/Textures/Biomes/Rock/Displacement.jpg");
    DisplacementPaths.push_back("../Resources/Textures/Biomes/Sand/Displacement.jpg");
    DisplacementPaths.push_back("../Resources/Textures/Biomes/Snow/Displacement.jpg");

    Displacement = loadTextureArray(DisplacementPaths);

    RoughnessPaths.push_back("../Resources/Textures/Biomes/Dirt/Roughness.jpg");
    RoughnessPaths.push_back("../Resources/Textures/Biomes/Forest/Roughness.jpg");
    RoughnessPaths.push_back("../Resources/Textures/Biomes/Grass/Roughness.jpg");
    RoughnessPaths.push_back("../Resources/Textures/Biomes/Rock/Roughness.jpg");
    RoughnessPaths.push_back("../Resources/Textures/Biomes/Sand/Roughness.jpg");
    RoughnessPaths.push_back("../Resources/Textures/Biomes/Snow/Roughness.jpg");

    Roughness = loadTextureArray(RoughnessPaths);
}

BiomeRenderer::~BiomeRenderer() {
    glDeleteVertexArrays(1, &planetVAO);
}

void BiomeRenderer::renderPlanet(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, glm::vec3 cameraPos, glm::mat4 model, float dt, std::shared_ptr<Shader> shader) {
    
    timeElapsed = dt;

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_CULL_FACE);

    shader->use();
    shader->setMat4("view", viewMatrix);
    shader->setMat4("projection", projectionMatrix);
    shader->setMat4("model", model);
    shader->setVec3("cameraPos", cameraPos);
    shader->setFloat("elapsedTime", timeElapsed);
    shader->setBool("sphere", g_guiData.isSphere);

    glBindVertexArray(planetVAO);

    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glDrawElements(GL_PATCHES, indexCount, GL_UNSIGNED_INT, 0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
}

void BiomeRenderer::renderWater(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, glm::vec3 cameraPos, glm::mat4 model, float dt) {

    timeElapsed = dt;

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    waterLightingShader->use();
    waterLightingShader->setMat4("view", viewMatrix);
    waterLightingShader->setMat4("projection", projectionMatrix);
    waterLightingShader->setMat4("model", model);
    waterLightingShader->setVec3("cameraPos", cameraPos);
    waterLightingShader->setFloat("elapsedTime", timeElapsed);
    waterLightingShader->setBool("sphere", g_guiData.isSphere);
    waterLightingShader->setBool("DuDv", g_guiData.isDuDv);

    glBindVertexArray(planetVAO);

    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);

    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glDrawElements(GL_PATCHES, indexCount, GL_UNSIGNED_INT, 0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
}

void BiomeRenderer::renderAtmosphere(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, glm::vec3 cameraPos, glm::mat4 model, float dt) {

    timeElapsed = dt;

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    atmosphereLightingShader->use();
    atmosphereLightingShader->setMat4("view", viewMatrix);
    atmosphereLightingShader->setMat4("projection", projectionMatrix);
    atmosphereLightingShader->setMat4("model", model);
    atmosphereLightingShader->setVec3("cameraPos", cameraPos);
    atmosphereLightingShader->setFloat("elapsedTime", timeElapsed);
    atmosphereLightingShader->setBool("sphere", g_guiData.isSphere);

    glBindVertexArray(planetVAO);

    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);

    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glDrawElements(GL_PATCHES, indexCount, GL_UNSIGNED_INT, 0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
}

void BiomeRenderer::renderMoon(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, glm::vec3 cameraPos, glm::mat4 model, float dt) {

    timeElapsed = dt;

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_CULL_FACE);

    moonLightingShader->use();
    moonLightingShader->setMat4("view", viewMatrix);
    moonLightingShader->setMat4("projection", projectionMatrix);
    moonLightingShader->setMat4("model", model);
    moonLightingShader->setVec3("cameraPos", cameraPos);
    moonLightingShader->setFloat("elapsedTime", timeElapsed);
    moonLightingShader->setBool("sphere", g_guiData.isSphere);

    glBindVertexArray(planetVAO);

    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glDrawElements(GL_PATCHES, indexCount, GL_UNSIGNED_INT, 0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
}

void BiomeRenderer::setupShaderWithMaps(GLuint heightMap, GLuint tempMap, GLuint precipMap, GLuint dudvMap, std::shared_ptr<Shader> shader) {
    shader->use();

    shader->setInt("heightMap", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, heightMap);

    shader->setInt("tempMap", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tempMap);

    shader->setInt("precipMap", 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, precipMap);

    shader->setInt("DuDvMap", 4);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, dudvMap);



    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D_ARRAY, Albedo);
    shader->setInt("AlbedoArray", 5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D_ARRAY, AO);
    shader->setInt("AOArray", 6);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D_ARRAY, Displacement);
    shader->setInt("DisplacementArray", 7);

    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D_ARRAY, Normal);
    shader->setInt("NormalArray", 8);

    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D_ARRAY, Roughness);
    shader->setInt("RoughnessArray", 9);
}
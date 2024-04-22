#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include <iostream>

#include "Scene_Data/Textures/stb_image.h"
#include "Shaders/Shader.h"

struct Cloud {
    glm::vec3 position;
};

class WeatherSystem {
public:
    WeatherSystem(const char* skyboxShaderFilepath, const char* cloudShaderFilepath);
    ~WeatherSystem();

    void renderSkybox(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    void renderClouds(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3 position);
    void loadSkybox(std::shared_ptr<Shader> shader);

private:
    GLuint cubemapTexture;
    GLuint skyboxVAO;
    GLuint skyboxVBO;

    GLuint cloudTextureArray;
    GLuint cloudVAO, cloudVBO;
    std::vector<Cloud> clouds;

    std::shared_ptr<Shader> skyboxShader;
    std::shared_ptr<Shader> cloudShader;

    GLuint loadCubemap(const std::vector<std::string>& faces);
    void setupSkybox();
    void setupClouds();
    void generateClouds(int count);
    GLuint loadTextureArray(std::vector<std::string> texArrayPaths);
};
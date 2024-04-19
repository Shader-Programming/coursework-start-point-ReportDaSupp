#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <iostream>

#include "Scene_Data/Textures/stb_image.h"
#include "Shaders/Shader.h"

class WeatherSystem {
public:
    WeatherSystem(const char* skyboxShaderFilepath);
    ~WeatherSystem();

    void renderSkybox(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    void loadSkybox(std::shared_ptr<Shader> shader);

private:
    GLuint cubemapTexture;
    GLuint skyboxVAO;
    GLuint skyboxVBO;
    std::shared_ptr<Shader> skyboxShader;

    GLuint loadCubemap(const std::vector<std::string>& faces);
    void setupSkybox();
};
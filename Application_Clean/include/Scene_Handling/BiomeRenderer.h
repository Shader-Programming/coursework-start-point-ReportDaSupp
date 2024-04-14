#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include "Scene_Data/Textures/stb_image.h"
#include "Shaders/Shader.h"

class BiomeRenderer {
public:
    BiomeRenderer(GLuint vao, const char* shaderPath);
    ~BiomeRenderer();

    void renderPlanet(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, int indexCount, glm::vec3 cameraPos, float dt);
    void setupShaderWithMaps(GLuint heightMap, GLuint tempMap, GLuint precipMap);

private:
    std::shared_ptr<Shader> lightingShader;
    GLuint planetVAO;

    GLuint sandTexture, snowTexture, forestTexture, grassTexture, beachTexture;

    float timeElapsed = 0;

    GLuint loadTexture(const char* filepath) {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(filepath, &width, &height, &nrChannels, 0);
        if (!data) {
            std::cerr << "Failed to load texture: " << filepath << std::endl;
            return 0;
        }

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set texture wrapping and filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload the texture to GPU
        GLenum format = GL_RGB;
        if (nrChannels == 4)
            format = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done

        return textureID;
    }
};
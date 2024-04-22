#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "Scene_Data/Textures/stb_image.h"
#include "Shaders/Shader.h"

class BiomeRenderer {
public:
    BiomeRenderer(GLuint vao, const char* tShaderPath, const char* tPBRShaderPath, const char* wShaderPath, const char* aShaderPath, const char* mShaderPath, int iCount);
    ~BiomeRenderer();

    void renderPlanet(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, glm::vec3 cameraPos, glm::mat4 model, float dt, std::shared_ptr<Shader> shader);
    std::shared_ptr<Shader> getPlanetShader() { return terrainLightingShader; };
    std::shared_ptr<Shader> getPBRPlanetShader() { return terrainPBRLightingShader; };
    void renderWater(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, glm::vec3 cameraPos, glm::mat4 model, float dt);
    std::shared_ptr<Shader> getWaterShader() { return waterLightingShader; };
    void renderAtmosphere(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, glm::vec3 cameraPos, glm::mat4 model, float dt);
    std::shared_ptr<Shader> getAtmosphereShader() { return atmosphereLightingShader; };
    void renderMoon(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, glm::vec3 cameraPos, glm::mat4 model, float dt);
    std::shared_ptr<Shader> getMoonShader() { return moonLightingShader; };
    void setupShaderWithMaps(GLuint heightMap, GLuint tempMap, GLuint precipMap, GLuint dudvMap, std::shared_ptr<Shader> shader);

private:
    std::shared_ptr<Shader> terrainLightingShader, terrainPBRLightingShader, waterLightingShader, atmosphereLightingShader, moonLightingShader;
    GLuint planetVAO;

    GLuint Albedo, AO, Displacement, Normal, Roughness;

    std::vector<const char*> AlbedoPaths, AOPaths, DisplacementPaths, NormalPaths, RoughnessPaths;

    float timeElapsed = 0;
    int indexCount = 0;

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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        GLenum format1, format2;

        if (nrChannels == 1)
            format1 = GL_R8;
        else if (nrChannels == 3)
            format1 = GL_RGB8;
        else if (nrChannels == 4)
            format1 = GL_RGBA8;

        if (nrChannels == 1)
            format2 = GL_RED;
        else if (nrChannels == 3)
            format2 = GL_RGB;
        else if (nrChannels == 4)
            format2 = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format1, width, height, 0, format2, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done

        return textureID;
    }

    GLuint loadTextureArray(std::vector<const char*> texArrayPaths) {
        int width, height, nrChannels;
        std::vector<unsigned char*> data;
        data.resize(texArrayPaths.size());
        for (int i = 0; i < texArrayPaths.size(); i++)
        {
            data[i] = stbi_load(texArrayPaths[i], &width, &height, &nrChannels, 0);
            if (!data[i]) {
                std::cerr << "Failed to load texture: " << texArrayPaths[i] << std::endl;
                return 0;
            }
        }

        GLuint textureArray;
        glGenTextures(1, &textureArray);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);

        GLenum format1, format2;

        if (nrChannels == 1)
            format1 = GL_R8;
        else if (nrChannels == 3)
            format1 = GL_RGB8;
        else if (nrChannels == 4)
            format1 = GL_RGBA8;

        if (nrChannels == 1)
            format2 = GL_RED;
        else if (nrChannels == 3)
            format2 = GL_RGB;
        else if (nrChannels == 4)
            format2 = GL_RGBA;

        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format1, width, height, texArrayPaths.size(), 0, format2, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        for (int i = 0; i < texArrayPaths.size(); ++i) {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, width, 1, format2, GL_UNSIGNED_BYTE, data[i]);
            glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        }
        

        for (int i = 0; i < texArrayPaths.size(); i++)
            stbi_image_free(data[i]);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done

        return textureArray;
    }
};
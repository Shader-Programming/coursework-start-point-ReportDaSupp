#include "Scene_Handling/WeatherSystem.h"
#include <iomanip>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>

WeatherSystem::WeatherSystem(const char* skyboxShaderFilepath, const char* cloudShaderFilepath) {
    skyboxShader = std::make_shared<Shader>(skyboxShaderFilepath);
    cloudShader = std::make_shared<Shader>(cloudShaderFilepath);
    std::vector<std::string> faces;
    for (int i = 0; i < 6; i++)
        faces.push_back((std::string("../Resources/Textures/SkyBox/" + std::to_string(i+1) + ".png")).c_str());
    cubemapTexture = loadCubemap(faces);
    setupSkybox();
    std::vector<std::string> texPaths;
    texPaths.resize(84);
    for (int i = 0; i < 84; i++) {
        std::stringstream ss;
        ss << "../Resources/Textures/Clouds/" << std::setw(4) << std::setfill('0') << i + 1 << ".png";
        texPaths[i] = ss.str();
    }
    loadTextureArray(texPaths);
    generateClouds(30);
    setupClouds();
}

WeatherSystem::~WeatherSystem() {
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
}

void WeatherSystem::renderSkybox(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    skyboxShader->use();
    skyboxShader->setMat4("view", glm::mat4(glm::mat3(viewMatrix)));
    skyboxShader->setMat4("projection", projectionMatrix);

    glBindVertexArray(skyboxVAO);
    
    skyboxShader->setInt("skybox", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}

void WeatherSystem::renderClouds(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3 position)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    cloudShader->use();
    glBindVertexArray(cloudVAO);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, cloudTextureArray);
    cloudShader->setInt("cloudArray", 0);
    cloudShader->setMat4("view", viewMatrix);
    cloudShader->setMat4("projection", projectionMatrix);
    cloudShader->setVec3("viewPosition", position);
    for (const auto& cloud : clouds) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), cloud.position);
        cloudShader->setMat4("model", model);
        glDrawArrays(GL_POINTS, 0, 1);
    }
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    
}

void WeatherSystem::loadSkybox(std::shared_ptr<Shader> shader)
{
    shader->setInt("skybox", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
}

GLuint WeatherSystem::loadCubemap(const std::vector<std::string>& faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (GLuint i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void WeatherSystem::setupSkybox() {
    float skyboxVertices[] = {
        // Positions          
        // Back face
        -1.0f, -1.0f, -1.0f, // Bottom-left
         1.0f,  1.0f, -1.0f, // top-right
         1.0f, -1.0f, -1.0f, // bottom-right         
         1.0f,  1.0f, -1.0f, // top-right
        -1.0f, -1.0f, -1.0f, // bottom-left
        -1.0f,  1.0f, -1.0f, // top-left

        // Front face
        -1.0f, -1.0f,  1.0f, // bottom-left
         1.0f, -1.0f,  1.0f, // bottom-right
         1.0f,  1.0f,  1.0f, // top-right
         1.0f,  1.0f,  1.0f, // top-right
        -1.0f,  1.0f,  1.0f, // top-left
        -1.0f, -1.0f,  1.0f, // bottom-left

        // Left face
        -1.0f,  1.0f,  1.0f, // top-right
        -1.0f,  1.0f, -1.0f, // top-left
        -1.0f, -1.0f, -1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, // top-right

        // Right face
         1.0f,  1.0f,  1.0f, // top-left
         1.0f, -1.0f, -1.0f, // bottom-right
         1.0f,  1.0f, -1.0f, // top-right         
         1.0f, -1.0f, -1.0f, // bottom-right
         1.0f,  1.0f,  1.0f, // top-left
         1.0f, -1.0f,  1.0f, // bottom-left

         // Bottom face
         -1.0f, -1.0f, -1.0f, // top-right
          1.0f, -1.0f, -1.0f, // top-left
          1.0f, -1.0f,  1.0f, // bottom-left
          1.0f, -1.0f,  1.0f, // bottom-left
         -1.0f, -1.0f,  1.0f, // bottom-right
         -1.0f, -1.0f, -1.0f, // top-right

         // Top face
         -1.0f,  1.0f, -1.0f, // top-left
          1.0f,  1.0f,  1.0f, // bottom-right
          1.0f,  1.0f, -1.0f, // top-right     
          1.0f,  1.0f,  1.0f, // bottom-right
         -1.0f,  1.0f, -1.0f, // top-left
         -1.0f,  1.0f,  1.0f  // bottom-left
    };

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void WeatherSystem::setupClouds()
{
    glGenVertexArrays(1, &cloudVAO);
    glBindVertexArray(cloudVAO);
    glGenBuffers(1, &cloudVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cloudVBO);
    glBufferData(GL_ARRAY_BUFFER, clouds.size() * sizeof(Cloud), clouds.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void WeatherSystem::generateClouds(int count)
{
    srand((unsigned)time(0));
    float planetRadius = 125.0f;
    for (int i = 0; i < count; ++i) {
        
        float latitude = static_cast<float>(rand()) / RAND_MAX * 3.14159;
        float longitude = static_cast<float>(rand()) / RAND_MAX * 2 * 3.14159;
        glm::vec3 position = glm::vec3(
            planetRadius * sin(latitude) * cos(longitude),
            planetRadius * sin(latitude) * sin(longitude),
            planetRadius * cos(latitude));
        clouds.push_back({ position});
    }
}

GLuint WeatherSystem::loadTextureArray(std::vector<std::string> texArrayPaths) {
    int width, height, nrChannels;
    std::vector<unsigned char*> data;
    data.resize(texArrayPaths.size());
    for (int i = 0; i < texArrayPaths.size(); i++)
    {
        data[i] = stbi_load(texArrayPaths[i].c_str(), &width, &height, &nrChannels, 0);
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
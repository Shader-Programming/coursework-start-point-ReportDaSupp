#include "Scene_Handling/WeatherSystem.h"

WeatherSystem::WeatherSystem(const char* skyboxShaderFilepath) {
    skyboxShader = std::make_shared<Shader>(skyboxShaderFilepath);
    std::vector<std::string> faces;
    for (int i = 0; i < 6; i++)
        faces.push_back((std::string("../Resources/Textures/SkyBox/" + std::to_string(i+1) + ".png")).c_str());
    cubemapTexture = loadCubemap(faces);
    setupSkybox();
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

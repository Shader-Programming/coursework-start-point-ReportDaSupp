#pragma once
#include "Scene_Data/Textures/TextureManager.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;

    // The Following is Additional Data Held By Assimp & Stored For Future Use
    glm::vec3 Bitangent;
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};

class Mesh {
public:
    // Mesh Data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    unsigned int VAO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(std::shared_ptr<Shader> shader);

private:
    unsigned int VBO, IBO;
    void setupMesh();
};

class Model
{
public:
    // Model Data 
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;
    bool isStatic;

    Model(std::string path, bool flip = false, bool isstatic = false, bool gamma = false);
    void setTransform(glm::mat4 model, glm::vec3 translation, glm::vec3 rotation = glm::vec3(0.0f), float rate = 1.f, glm::vec3 scale = glm::vec3(1.0f));
    glm::mat4 getTransform() { return modelTransform; };
    void Draw(std::shared_ptr<Shader> shader);

private:
    void loadModel(std::string const& path, bool flip);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    glm::mat4 modelTransform;
};
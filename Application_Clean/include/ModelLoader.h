#pragma once

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "glm/common.hpp"
#include <memory>

#include "Model.h"


class ModelLoader {
public:
	ModelLoader();
	std::shared_ptr<Model> getModel(const char* path);

private:
	Assimp::Importer importer;
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	void processNode(aiNode* node, const aiScene* scene);
	void loadModel(const char* path);

	std::vector<Mesh> m_meshes;
	std::vector<Texture*> m_textures;
	std::string m_directory;

	Texture* diff;
	Texture* spec;
	Texture* norm;






};
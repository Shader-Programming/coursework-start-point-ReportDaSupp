#include "ModelLoader.h"
#include <iostream>


ModelLoader::ModelLoader()
{


}

std::shared_ptr<Model> ModelLoader::getModel(const char* path)
{
	// clear previous (if any model details)
	m_textures.clear();
	m_meshes.clear();

	//load model
	loadModel(path);

	m_textures.push_back(diff);
	m_textures.push_back(spec);
	m_textures.push_back(norm);


	return std::make_shared<Model>(m_meshes, m_textures);

}

void ModelLoader::loadModel(const char* path)
{
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	// retrieve the directory path of the filepath
	std::string rootPath = path;
	m_directory = rootPath.substr(0, rootPath.find_last_of('\\'));

	m_meshes.clear();
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}

	processNode(scene->mRootNode, scene);
}



void ModelLoader::processNode(aiNode* node, const aiScene* scene)
{
	// models can contain more than a single mesh, eg mesh for arm, another for torso, etc.
	// so loop through each
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node represents relationships in scene - important for animating models
		// retrieve the Assimp mesh
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		// process data
		m_meshes.push_back(processMesh(mesh, scene));
	}

	// recursive, through all children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

// here we extract vertex and texture data
// no error handling or checking
// make sure the model you are loading has normals, textures, etc
Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene)
{
	Vertex vertex;
	Mesh tmpMesh;

	glm::vec3 vector3;
	glm::vec2 vector2;

	// loop through each vertex in mesh
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		if (mesh->HasPositions()) {

			vertex.position.x = mesh->mVertices[i].x;
			vertex.position.y = mesh->mVertices[i].y;
			vertex.position.z = mesh->mVertices[i].z;

		}

		if (mesh->HasNormals()) {

			vertex.normal.x = mesh->mNormals[i].x;
			vertex.normal.y = mesh->mNormals[i].y;
			vertex.normal.z = mesh->mNormals[i].z;

		}
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			// mesh can contain more than one set of UVs, we'll only take first set. Update this if you need more!
			vertex.uv.x = mesh->mTextureCoords[0][i].x;
			vertex.uv.y = mesh->mTextureCoords[0][i].y;

		}
		if (mesh->HasTangentsAndBitangents()) {
			// tan only - can calculate bitan with tan and normal
			vertex.tangent.x = mesh->mTangents[i].x;
			vertex.tangent.y = mesh->mTangents[i].y;
			vertex.tangent.z = mesh->mTangents[i].z;

		}


		tmpMesh.addVertex(vertex);
	}

	// indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			tmpMesh.addIndex(face.mIndices[j]);
	}

	/*
	Read Textures - code below assumes that there is at least a diffuse, specular and normal map
	There is NO ERROR HANDELING. Add this to make your code more robust. At least add an assert.
	What happens if there is only diffuse and normal? Below will break.
	EXTEND THIS CODE
	As if is, the below will fill std::vector m_textures with
	[0] diffuse
	[1] specular
	[2] normal
	*/






	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	aiString str;
	std::string pathToTexture;
	//DIFFUSE
	if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
		material->GetTexture(aiTextureType_DIFFUSE, 0, &str); // 0 , only one type of texture per mesh allowed so far.	
		pathToTexture = m_directory + "\\" + str.C_Str();
		diff = new Texture(pathToTexture.c_str());
	}
	// SPEC
	if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
		material->GetTexture(aiTextureType_SPECULAR, 0, &str); // 0 , only one type of texture per mesh allowed so far.
		pathToTexture = m_directory + "\\" + str.C_Str();
		spec = new Texture(pathToTexture.c_str());
	}
	// NORMAL
	if (material->GetTextureCount(aiTextureType_HEIGHT) > 0) {
		material->GetTexture(aiTextureType_HEIGHT, 0, &str); // 0 , only one type of texture per mesh allowed so far.
		pathToTexture = m_directory + "\\" + str.C_Str();
		norm = new Texture(pathToTexture.c_str());
	}



	return tmpMesh;

}
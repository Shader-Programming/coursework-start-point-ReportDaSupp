
#include "Model.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>


Model::Model(std::vector<Mesh> meshes, std::vector<Texture*> textures) : m_meshes(meshes), m_textures(textures)
{
	createBuffers();
}

void Model::createBuffers()
{
	// Should really be passing data to renderer to create buffers, but we don't have one ( yet!) so create buffers here.
	// call create buffers on mesh class
	// Will need to do this for every mesh in model, but we are only dealing with first ( or only mesh) at the moment
	// m_meshes is a vector of meshes, so take first one [0]
	m_meshes[0].createBuffers();
}






void Model::renderModel(std::shared_ptr<Shader>& shader)
{
	shader->use();

	//shader->setMat4("model", m_model); // transform matrix

	// bind textures - you could move this to a function?
	//diffuse
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textures[0]->getID());
	shader->setInt("diffuseTexture", 0);

	//spec
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_textures[1]->getID());
	shader->setInt("specTexture", 1);

	//norm
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_textures[2]->getID());
	shader->setInt("normalTexture", 2);



	// loop through each mesh and draw or in our case, for now, only one mesh so draw first one:
	m_meshes[0].drawMesh();
}










#pragma once
#include <glm/detail/type_mat.hpp>
#include <Shader.h>
#include <vector>

class SkyBox {

public:
	SkyBox();
	void renderSkyBox(glm::mat4 cameraMatrix);

private:
	Shader* skyBoxShader;
	void createSkyBox();
	void loadCubeMapTextures();
	unsigned int loadCubemap(std::vector<std::string> faces);
	void createVAO();

	std::vector<std::string> m_faces;
	unsigned int skyBoxTexObj;
	unsigned int skyBoxVAO;

	const float skyBoxVertices[24] = {};

	const unsigned int skyBoxindices[36] = {};


};
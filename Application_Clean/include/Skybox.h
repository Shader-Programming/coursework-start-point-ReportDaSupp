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

	const float skyboxVertices[24] = {       
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f
	};

	const unsigned int skyboxIndices[36] = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4,
	7, 3, 0, 0, 4, 7,
	1, 5, 6, 6, 2, 1,
	4, 5, 1, 1, 0, 4,
	7, 6, 2, 2, 3, 7
	};


};
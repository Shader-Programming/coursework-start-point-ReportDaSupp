#include "Skybox.h"
#include <stb_image.h>
#include <iostream>

SkyBox::SkyBox() {
	skyBoxShader = new Shader("..\\Shaders\\SkyBox.glsl");
	this->createSkyBox();
}

void SkyBox::renderSkyBox(glm::mat4 cameraMatrix)
{
	skyBoxShader->use();
	skyBoxShader->setMat4("CameraMatrix", cameraMatrix);
	skyBoxShader->setInt("SkyTexture", 0);
	glDepthMask(GL_FALSE);
	glBindVertexArray(skyBoxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexObj);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glDepthMask(GL_TRUE);
}

void SkyBox::createSkyBox() {
	loadCubeMapTextures();
	createVAO();
}

void SkyBox::loadCubeMapTextures() {
	m_faces.push_back("..\\resources\\SkyBox\\right.png");
	m_faces.push_back("..\\resources\\SkyBox\\left.png");
	m_faces.push_back("..\\resources\\SkyBox\\top2.png");
	m_faces.push_back("..\\resources\\SkyBox\\bottom.png");
	m_faces.push_back("..\\resources\\SkyBox\\back.png");
	m_faces.push_back("..\\resources\\SkyBox\\front.png");
	skyBoxTexObj = loadCubemap(m_faces);
}

unsigned int SkyBox::loadCubemap(std::vector<std::string> faces) {

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            //std::cout << "Loaded texture at path: " << faces[i] << " width " << width << " id " << textureID << std::endl;
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
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

void SkyBox::createVAO()
{
	unsigned int VBO, EBO;

	glGenVertexArrays(1, &skyBoxVAO);

	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(skyBoxVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);

	//xyz
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

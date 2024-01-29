#include "Scene_Data/Objects/Cube.h"

Cube::Cube(glm::vec3 Color, float Shine, float SpecStrength, const char* dfilepath, const char* sfilepath, const char* nfilepath) : m_color(Color), m_shine(Shine), m_specularStrength(SpecStrength)
{
	this->makeVAO();
	this->resetTransform();
	diffuseTex = g_TextureManager.assignTexture(dfilepath, "diffuseTexture");
	specularTex = g_TextureManager.assignTexture(sfilepath, "specularTexture");
	normalTex = g_TextureManager.assignTexture(nfilepath, "normalTexture");
}

void Cube::setMaterialValues(std::shared_ptr<Shader> Shader)
{
	Shader->setMat4("model", m_transform);
	Shader->setVec3("cubeColor", m_color);
}

void Cube::render()
{
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, vertexData.size(), GL_UNSIGNED_INT, 0);
}

void Cube::makeVAO()
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_IBO);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexData.size(), indexData.data(), GL_STATIC_DRAW);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexData.size(), vertexData.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);
}

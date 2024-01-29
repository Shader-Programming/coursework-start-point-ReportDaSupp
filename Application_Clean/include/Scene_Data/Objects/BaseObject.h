#pragma once
#include "Shaders/Shader.h"
#include <memory>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

/*
Base Class For Object Definitions
*/
class BaseObject
{
public:
	BaseObject() = default;
	~BaseObject() {};

	virtual void setMaterialValues(std::shared_ptr<Shader> Shader) = 0;
	virtual void bindTextures() = 0;
	virtual void render() = 0;
	void resetTransform() { m_transform = glm::mat4(1.0f); }

	virtual uint32_t getVAO() { return m_VAO; };
	virtual glm::mat4& getTransform() { return m_transform; };
	virtual std::vector<float> getVertices() { return vertexData; };
	virtual std::vector<uint32_t> getIndices() { return indexData; };

	virtual void translate(glm::vec3 translation) { m_transform = glm::translate(m_transform, translation); };
	virtual void rotate(float angle, glm::vec3 axis) { m_transform = glm::rotate(m_transform, angle, axis); };
	virtual void scale(float scaleFactor, glm::vec3 axis) { m_transform = glm::scale(m_transform, (scaleFactor * axis)); };


private:
	glm::mat4 m_transform;
	glm::vec3 m_color;

	uint32_t m_VAO, m_VBO, m_IBO;

	float m_shine, m_specularStrength;

	std::vector<float> vertexData;
	std::vector<uint32_t> indexData;

};

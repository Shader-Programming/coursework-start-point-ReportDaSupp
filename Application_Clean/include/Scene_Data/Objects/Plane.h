#pragma once
#include "Scene_Data/Objects/BaseObject.h"
#include "Scene_Data/Textures/TextureManager.h"

/*
Inherited Class For A Cube Object
*/
class Plane : public BaseObject
{
public:
	Plane();
	~Plane() {};

	virtual void setMaterialValues(std::shared_ptr<Shader> Shader) override;
	virtual void bindTextures() override {};
	virtual void render() override;

	virtual uint32_t getVAO() override { return m_VAO; };
	virtual glm::mat4& getTransform() override { return m_transform; };
	virtual std::vector<float> getVertices() override { return vertexData; };
	virtual std::vector<uint32_t> getIndices() override { return indexData; };

	virtual void translate(glm::vec3 translation) override { m_transform = glm::translate(m_transform, translation); };
	virtual void rotate(float angle, glm::vec3 axis) override { m_transform = glm::rotate(m_transform, angle, axis); };
	virtual void scale(float scaleFactor, glm::vec3 axis) override { m_transform = glm::scale(m_transform, (scaleFactor * axis)); };

private:
	void makeVAO();

	glm::mat4 m_transform;
	glm::vec3 m_color;
	std::shared_ptr<Texture> ColorTex;
	std::shared_ptr<Texture> DepthTex;

	uint32_t m_VAO, m_VBO, m_IBO;

	std::vector<float> vertexData = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,

	};

	std::vector<uint32_t> indexData = {
		3,2,1,
		3,1,0
	};

};


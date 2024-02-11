#pragma once
#include "Scene_Data/Objects/BaseObject.h"
#include "Scene_Data/Textures/TextureManager.h"

/*
Inherited Class For A Cube Object
*/
class Cube : public BaseObject
{
public:
	Cube(glm::vec3 Color, float Shine, float SpecStrength, const char* dfilepath = (const char*)("..\\Resources\\Textures\\diffuseCube.jpg"), const char* sfilepath = (const char*)("..\\Resources\\Textures\\specularCube.jpg"), const char* nfilepath = (const char*)("..\\Resources\\Textures\\normalCube.jpg"));
	~Cube() {};

	virtual void setMaterialValues(std::shared_ptr<Shader> Shader) override;
	virtual void bindTextures() override {};
	virtual void render() override;

	virtual uint32_t getVAO() override { return m_VAO; };
	virtual glm::mat4& getTransform() override { return m_transform; };
	virtual std::vector<float> getVertices() override { return vertexData; };
	virtual std::vector<uint32_t> getIndices() override { return indexData; };

	virtual void setTransform(glm::vec3 T, glm::vec3 R, glm::vec3 S) override
	{
		glm::mat4 t = glm::translate(glm::mat4(1.0f), T);
		glm::mat4 r = glm::mat4_cast(glm::quat(R));
		glm::mat4 s = glm::scale(glm::mat4(1.0f), S);
		m_transform = t * r * s;
	}

	virtual void translate(glm::vec3 translation) override { m_transform = glm::translate(m_transform, translation); };
	virtual void rotate(float angle, glm::vec3 axis) override { m_transform = glm::rotate(m_transform, angle, axis); };
	virtual void scale(float scaleFactor, glm::vec3 axis) override { m_transform = glm::scale(m_transform, (scaleFactor * axis)); };

private:
	void makeVAO();

	glm::mat4 m_transform;
	glm::vec3 m_color;
	std::shared_ptr<Texture> diffuseTex;
	std::shared_ptr<Texture> specularTex;
	std::shared_ptr<Texture> normalTex;


	uint32_t m_VAO, m_VBO, m_IBO;

	float m_shine, m_specularStrength;

	std::vector<float> vertexData = {
		//  xyz, normal,uv, tan
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,1.0f, 0.0f, 0.0f,
		// front
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,-1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,-1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,-1.0f, 0.0f, 0.0f,
		// left
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,0.0f, 0.0f, 1.0f,
		// right
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,0.0f, 0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,0.0f, 0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,0.0f, 0.0f, -1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,0.0f, 0.0f, -1.0f,
		 // bottom
		 -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,1.0f, 0.0f, 0.0f,
		  0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,1.0f, 0.0f, 0.0f,
		  0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,1.0f, 0.0f, 0.0f,
		 -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,1.0f, 0.0f, 0.0f,
		 // top
		 -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,1.0f, 0.0f, 0.0f,
		  0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,1.0f, 0.0f, 0.0f,
		  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,1.0f, 0.0f, 0.0f,
		 -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,1.0f, 0.0f, 0.0f
	};
	std::vector<unsigned int> indexData = {
	0, 1, 2,  2, 3, 0,
	4, 5, 6,  6, 7, 4,
	8, 9, 10, 10, 11, 8,
	12, 13, 14, 14, 15, 12,
	16, 17, 18, 18, 19, 16,
	20, 21, 22, 22, 23, 20
	};

};

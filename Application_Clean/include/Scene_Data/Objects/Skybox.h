#pragma once
#include "Scene_Data/Objects/BaseObject.h"
#include "Scene_Data/Textures/TextureManager.h"

/*
Inherited Class For A Cube Object
*/
class SkyBox : public BaseObject
{
public:
	SkyBox(const char* dfilepath = (const char*)("..\\Resources\\Textures\\Skybox\\"));
	~SkyBox() {};

	virtual void setMaterialValues(std::shared_ptr<Shader> Shader) override;
    virtual void bindTextures() override {};
	virtual void render() override;

	virtual uint32_t getVAO() override { return m_VAO; };
	virtual glm::mat4& getTransform() override { return m_transform; };
	virtual std::vector<float> getVertices() override { return vertexData; };
	virtual std::vector<uint32_t> getIndices() override { return std::vector<unsigned int>(); };

	virtual void translate(glm::vec3 translation) override { m_transform = glm::translate(m_transform, translation); };
	virtual void rotate(float angle, glm::vec3 axis) override { m_transform = glm::rotate(m_transform, angle, axis); };
	virtual void scale(float scaleFactor, glm::vec3 axis) override { m_transform = glm::scale(m_transform, (scaleFactor * axis)); };

private:
	void makeVAO();
	uint32_t loadCubemap(std::vector<std::string> faces);

	glm::mat4 m_transform;
    std::shared_ptr<Texture> skyboxTex;

	uint32_t m_VAO, m_VBO, m_IBO;

	std::vector<float> vertexData = {
		//  xyz, normal,uv, tan
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
	};

};

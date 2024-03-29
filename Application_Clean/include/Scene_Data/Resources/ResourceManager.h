#pragma once
#include "Scene_Data/Lights/LightsBase.h"
#include "Scene_Data/Objects/Cube.h"
#include "Scene_Data/Objects/Skybox.h"
#include "Scene_Data/Objects/AssimpObj.h"
#include <unordered_map>

struct SimpleVertex
{
	glm::vec4 position;
	glm::vec4 texCoords;
	glm::vec4 normal;
};

struct Resources {
	DirectionalLight m_directionalLight;
	std::vector<PointLight> m_pointLights;
	std::vector<SpotLight> m_spotLights;
	std::vector<std::shared_ptr<BaseObject>> m_sceneObjects;
	std::vector<Cube> m_lights;
	std::vector<Model> m_models;
	std::shared_ptr<SkyBox> m_SkyBox;
	uint32_t HeightMapTexture;
	
	uint32_t m_terrainSSBO;
	uint32_t m_terrainVAO;
	uint32_t m_terrainVBO;
	uint32_t m_terrainCount;
	std::vector<uint32_t> m_terrainIndices;

	SpotLight m_torch;

	std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;

	bool eWireframe = false;
	bool eBloom = true;
	bool eTone = true;
	bool eGrayscale = false;
	bool eInverse = false;
	bool eGamma = true;
	bool eDirectional = true;
	bool ePointLight = false;
	bool eSpotLight = true;
	bool eRimming = false;
	bool eLightsVisible = false;
	bool eDirectionalSM = false;
	bool eNormalMap = false;

	float ellapsedTime = 0.f;

};

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager() {};
	std::shared_ptr<Resources> getResourcesPtr() { return resources; }

	void initAssimpModel(const char* fp, bool isFlip, bool isStatic, glm::vec3 trans, glm::vec3 rotate, glm::vec3 scale);
	void initTerrain(uint32_t gridWidth, uint32_t gridHeight);
	void initSkyBox(const char* fp);
	void initSceneObject(std::shared_ptr<BaseObject> obj, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
	void initDirectionalLight(glm::vec3 color, glm::vec3 direction, float ambient);
	void initPointLight(glm::vec3 color, glm::vec3 position, glm::vec3 attenuation);
	void initSpotLight(glm::vec3 color, glm::vec3 position, glm::vec3 direction, glm::vec3 attenuation, float cutOff, float outerCutoff);

private:

	std::shared_ptr<Resources> resources;
};
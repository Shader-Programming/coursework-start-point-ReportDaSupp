#pragma once
#include "Scene_Data/Lights/LightsBase.h"
#include "Scene_Data/Objects/Cube.h"
#include "Scene_Data/Objects/Skybox.h"
#include "Scene_Data/Objects/AssimpObj.h"
#include <unordered_map>

struct Resources {
	DirectionalLight m_directionalLight;
	std::vector<PointLight> m_pointLights;
	std::vector<SpotLight> m_spotLights;
	std::vector<std::shared_ptr<BaseObject>> m_sceneObjects;
	std::vector<Cube> m_lights;
	std::vector<Model> m_models;
	std::shared_ptr<Model> m_terrain;

	SpotLight m_torch;

	std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;

	bool eWireframe = false;
	bool eBloom = true;
	bool eTone = true;
	bool eGrayscale = false;
	bool eInverse = false;
	bool eGamma = false;
	bool eDirectional = true;
	bool ePointLight = true;
	bool eSpotLight = true;
	bool eRimming = false;
	bool eLightsVisible = true;
	bool eDirectionalSM = true;

};

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager() {};
	std::shared_ptr<Resources> getResourcesPtr() { return resources; }

	void initAssimpModel(const char* fp, bool isFlip, bool isStatic, glm::vec3 trans, glm::vec3 rotate, glm::vec3 scale);
	void initTerrain(const char* fp, bool isFlip, bool isStatic, glm::vec3 trans, glm::vec3 rotate, glm::vec3 scale);
	void initSceneObject(std::shared_ptr<BaseObject> obj, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
	void initDirectionalLight(glm::vec3 color, glm::vec3 direction, float ambient);
	void initPointLight(glm::vec3 color, glm::vec3 position, glm::vec3 attenuation);
	void initSpotLight(glm::vec3 color, glm::vec3 position, glm::vec3 direction, glm::vec3 attenuation, float cutOff, float outerCutoff);

private:

	std::shared_ptr<Resources> resources;
};
#include "Scene_Data/Resources/ResourceManager.h"
#include <time.h> 

ResourceManager::ResourceManager()
{
	resources.reset(new Resources);

	resources->m_shaders["TerrainShader"] = std::make_shared<Shader>("..\\Shaders\\Scene\\TerrainPass.glsl");
	resources->m_shaders["GeometryShader"]	=	std::make_shared<Shader>("..\\Shaders\\Scene\\GeometryPass.glsl");
	resources->m_shaders["LightShader"]		=	std::make_shared<Shader>("..\\Shaders\\Scene\\Light.glsl");
	resources->m_shaders["LightingShader"]	=	std::make_shared<Shader>("..\\Shaders\\Scene\\PBRLightingShader.glsl");
	resources->m_shaders["PPShader"]		=	std::make_shared<Shader>("..\\Shaders\\Post Processing\\HDRShader.vert",	"..\\Shaders\\Post Processing\\HDRShader.frag");
	resources->m_shaders["DShadowMapShader"]	=	std::make_shared<Shader>("..\\Shaders\\Post Processing\\DShadowMap.vert",	"..\\Shaders\\Post Processing\\DShadowMap.frag");

	//this->initAssimpModel("../Resources/Models/Backpack/backpack.obj", false, true, glm::vec3(2.0f, 2.0f, 0.0f), glm::vec3(0.f, 1.f, 0.f), 0.0f, glm::vec3(1.f, 1.f, 1.f));
	//this->initAssimpModel("../Resources/Models/Frog/object.obj", true, true, glm::vec3(-2.0f, 0.1f, 0.0f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(3.f, 3.f, 3.f));
	this->initTerrain("../Resources/Models/Plane/Plane.obj", true, false, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(200.f, 1.f, 200.f));
	//this->initAssimpModel("../Resources/Models/Floor/object.obj", true, false, glm::vec3(0.0f, 10.0f, -10.0f), glm::vec3(1.f, 0.f, 0.f), 90.f, glm::vec3(10.f, 0.001f, 10.f));

	this->initDirectionalLight(glm::vec3(0.2, 0.2, 0.2), glm::vec3(-0.5f, -1.0f, -0.5f), 0.2f);

	srand(time(NULL));

	for (int i = 0; i < 1; i++)
	{
		this->initPointLight(glm::vec3(((float)std::rand() / (RAND_MAX)), ((float)std::rand() / (RAND_MAX)), ((float)std::rand() / (RAND_MAX))), glm::vec3((((float)std::rand() / (RAND_MAX)) * 20) - 10, (((float)std::rand() / (RAND_MAX)) * 4), (((float)std::rand() / (RAND_MAX)) * 20) - 10), glm::vec3(1, 0.03, 0.0015));
	}

	this->initSpotLight(glm::vec3(0.3, 0.3, 0.3), glm::vec3(0.0, 10.0, 0.0), glm::vec3(0.0, -1.0, 0.0), glm::vec3(1.0, 0.03, 0.0015), (float) glm::cos(glm::radians(12.5f)), (float)glm::cos(glm::radians(17.5f)));

	for (auto light : resources->m_pointLights)
	{
		std::shared_ptr<Cube> temp = std::make_shared<Cube>(light.color, 16.f, 0.8f);
		this->initSceneObject(temp, light.position, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.5f, 0.5f, 0.5f));
	}
}

void ResourceManager::initAssimpModel(const char* fp, bool isFlip, bool isStatic, glm::vec3 trans, glm::vec3 rotate, glm::vec3 scale)
{
	Model temp(fp, isFlip);
	temp.setTransform(trans, rotate, scale);
	temp.isStatic = isStatic;
	resources->m_models.push_back(temp);
}

void ResourceManager::initTerrain(const char* fp, bool isFlip, bool isStatic, glm::vec3 trans, glm::vec3 rotate, glm::vec3 scale)
{
	resources->m_terrain = std::make_shared <Model>(fp, isFlip);
	resources->m_terrain->setTransform(trans, rotate, scale);
	resources->m_terrain->isStatic = isStatic;
}

void ResourceManager::initSceneObject(std::shared_ptr<BaseObject> obj, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	obj->setTransform(position, rotation, scale);
	resources->m_sceneObjects.push_back(obj);
	
}

void ResourceManager::initDirectionalLight(glm::vec3 color, glm::vec3 direction, float ambient)
{
	DirectionalLight tempDirectionalLight;
	tempDirectionalLight.color = color;
	tempDirectionalLight.direction = direction;
	tempDirectionalLight.ambient = ambient;
	resources->m_directionalLight = (tempDirectionalLight);
}

void ResourceManager::initPointLight(glm::vec3 color, glm::vec3 position, glm::vec3 attenuation)
{
	PointLight tempPointLight;
	tempPointLight.color = color;
	tempPointLight.position = position;
	tempPointLight.attenuation = attenuation;
	resources->m_pointLights.push_back(tempPointLight);
}

void ResourceManager::initSpotLight(glm::vec3 color, glm::vec3 position, glm::vec3 direction, glm::vec3 attenuation, float cutOff, float outerCutoff)
{
	SpotLight tempSpotLight;
	tempSpotLight.color = color;
	tempSpotLight.position = position;
	tempSpotLight.direction = direction;
	tempSpotLight.attenuation = attenuation;
	tempSpotLight.cutOff = cutOff;
	tempSpotLight.outerCutOff = outerCutoff;
	resources->m_spotLights.push_back(tempSpotLight);
}

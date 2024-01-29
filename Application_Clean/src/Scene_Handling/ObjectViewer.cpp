#include "Scene_Handling/ObjectViewer.h"
#include <math.h>
#include <stdlib.h>
#include <time.h> 

ObjectViewer::ObjectViewer(GLFWwindow* window, std::shared_ptr<InputHandler> H) : Scene(window, H)
{
	m_camera = std::make_shared<FirstPersonCamera>();
	m_camera->attachHandler(m_window, m_handler);

	m_GeometryShader = std::make_shared<Shader>("..\\Shaders\\Scene\\GeometryPass.vert", "..\\Shaders\\Scene\\GeometryPass.frag");
	m_LightShader = std::make_shared<Shader>("..\\Shaders\\Scene\\Light.vert", "..\\Shaders\\Scene\\Light.frag");
	m_LightingShader = std::make_shared<Shader>("..\\Shaders\\Scene\\LightingShader.vert", "..\\Shaders\\Scene\\LightingShader.frag");
	m_PPShader = std::make_shared<Shader>("..\\Shaders\\Post Processing\\HDRShader.vert", "..\\Shaders\\Post Processing\\HDRShader.frag");
	m_DShadowMapShader = std::make_shared<Shader>("..\\Shaders\\Post Processing\\DShadowMap.vert", "..\\Shaders\\Post Processing\\DShadowMap.frag");

	m_ColorFBO.initAttachments();
	m_ColorFBO.attachLightingShader(m_LightingShader);
	m_ColorFBO.attachColorShader(m_PPShader);

	initLights();
	initModels();
}

ObjectViewer::~ObjectViewer()
{

}

void ObjectViewer::update(float dt)
{
	m_ColorFBO.bindDefault();
	m_ColorFBO.clearScreen();

	m_camera->update(dt);

	for (auto& model : models)
		model.setTransform(model.getTransform(), glm::vec3(0.0f), glm::vec3(0.0f, 1.f, 0.0f), dt*45.f);

	if (m_camera->isTorch)
	{
		m_spotLights[0].position = m_camera->getPosition();
		m_spotLights[0].direction = m_camera->getFront();
	}

	this->render(dt);
}

void ObjectViewer::render(float dt)
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Main rendering pass
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	m_GeometryShader->use();
	m_ColorFBO.bindGBuffer();
	m_ColorFBO.clearScreen();

	// Geometry Rendering
	this->setMatrixUniforms(m_GeometryShader);
	this->renderGeometry(m_GeometryShader, dt);

	glDisable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	
	if (m_camera->eDirectionalSM)
	{
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		m_DShadowMapShader->use();
		m_ColorFBO.bindDSMFBO();
		m_ColorFBO.clearDepthBuffer();

		this->setSMUniforms(m_DShadowMapShader);
		this->renderGeometry(m_DShadowMapShader, dt);
	}

	glDisable(GL_DEPTH_TEST);

	// Lighting Pass
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	m_ColorFBO.bindFBO();
	m_LightingShader->use();
	m_ColorFBO.clearBuffer();
	this->setMatrixUniforms(m_LightingShader);
	if (m_camera->eDirectionalSM)
		this->setSMUniforms(m_LightingShader);
	this->setLightingUniforms(m_LightingShader);
	m_ColorFBO.calcLighting();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//m_ColorFBO.bindFBO();
	// Lighting Rendering as Cubes
	m_LightShader->use();
	this->setMatrixUniforms(m_LightShader);
	this->renderLighting(m_LightShader, dt);

	if (m_camera->eBloom)
		m_ColorFBO.drawSampledBloom();

	m_ColorFBO.bindDefault();
	m_PPShader->use();
	m_ColorFBO.clearBuffer();
	this->setPPShaderUniforms(m_ColorFBO.getActiveShader());
	m_ColorFBO.drawFrame();
}

void ObjectViewer::renderGeometry(std::shared_ptr<Shader> Shader, float dt)
{

	Shader->use();
	for (auto& model : models)
	{
		Shader->setMat4("model", model.getTransform());
		model.Draw(Shader);
	}

	for (auto& model : statics)
	{
		Shader->setMat4("model", model.getTransform());
		model.Draw(Shader);
	}

}

void ObjectViewer::renderLighting(std::shared_ptr<Shader> Shader, float dt)
{
	Shader->use();

	Shader->setBool("eLightsVisible", m_camera->eLightsVisible);

	if (!m_camera->eLightsVisible)
	{
		glEnable(GL_BLEND);
	}

	for (auto& Light : lights)
	{
		Light->setMaterialValues(Shader);
		Light->render();
	}

	if (!m_camera->eLightsVisible)
		glDisable(GL_BLEND);

}

void ObjectViewer::setMatrixUniforms(std::shared_ptr<Shader> Shader)
{
	m_projection = m_camera->getProjectionMatrix();
	m_view = m_camera->getViewMatrix();

	Shader->setMat4("view", m_view);
	Shader->setMat4("projection", m_projection);
	Shader->setVec3("viewPos", m_camera->getPosition());
}

void ObjectViewer::setSMUniforms(std::shared_ptr<Shader> Shader)
{
	glm::vec3 lightPos = glm::vec3((5, 20, 5));
	glm::vec3 lightDir = m_directionalLights[0].direction;

	float left = -15.0f;
	float right = 15.0f;
	float bottom = -15.0f;
	float top = 15.0f;
	float near_plane = 0.1f;
	float far_plane = 30.f;

	glm::mat4 lightProjection = glm::ortho(left, right, bottom, top, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightDir, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;
	Shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
}

void ObjectViewer::setLightingUniforms(std::shared_ptr<Shader> Shader)
{
	Shader->setBool("eDirectional", m_camera->eDirectional);
	Shader->setBool("ePointLight", m_camera->ePointLight);
	Shader->setBool("eSpotLight", m_camera->eSpotLight);
	Shader->setBool("eRimming", m_camera->eRimming);
	Shader->setBool("eDirectionalSM", m_camera->eDirectionalSM);

	Shader->setVec3("dLight.color", m_directionalLights[0].color);
	Shader->setVec3("dLight.direction", m_directionalLights[0].direction);
	
	std::string name;

	for (int i = 0; i < m_pointLights.size(); i++)
	{
		name = std::string("pLight.color[" + std::to_string(i) + "]");
		Shader->setVec3(name, m_pointLights[i].color);
		name = std::string("pLight.position[" + std::to_string(i) + "]");
		Shader->setVec3(name, m_pointLights[i].position);
		name = std::string("pLight.attenuation[" + std::to_string(i) + "]");
		Shader->setVec3(name, m_pointLights[i].attenuation);
	}

	Shader->setFloat("pLight.numLights", m_pointLights.size());

	for (int i = 0; i < m_spotLights.size(); i++)
	{
		name = std::string("sLight.color["+ std::to_string(i) + "]");
		Shader->setVec3(name, m_spotLights[i].color);
		name = std::string("sLight.position[" + std::to_string(i) + "]");
		Shader->setVec3(name, m_spotLights[i].position);
		name = std::string("sLight.direction[" + std::to_string(i) + "]");
		Shader->setVec3(name, m_spotLights[i].direction);
		name = std::string("sLight.attenuation[" + std::to_string(i) + "]");
		Shader->setVec3(name, m_spotLights[i].attenuation);
		name = std::string("sLight.cutOff[" + std::to_string(i) + "]");
		Shader->setFloat(name, m_spotLights[i].cutOff);
		name = std::string("sLight.outerCutOff[" + std::to_string(i) + "]");
		Shader->setFloat(name, m_spotLights[i].outerCutOff);
	}

	Shader->setFloat("sLight.enabled", m_camera->isTorch);
	Shader->setFloat("sLight.numLights", m_spotLights.size());
}

void ObjectViewer::setPPShaderUniforms(std::shared_ptr<Shader> Shader)
{
	Shader->setBool("eBloom", m_camera->eBloom);
	Shader->setBool("eTone", m_camera->eTone);
	Shader->setBool("eInverse", m_camera->eInverse);
	Shader->setBool("eGrayscale", m_camera->eGrayscale);
	Shader->setBool("eGamma", m_camera->eGamma);
}

void ObjectViewer::initModels()
{
		Model Backpack("../Resources/Backpack/backpack.obj");
		Backpack.setTransform(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 0.0f), glm::vec3(0.f, 1.f, 0.f), 0.0f);
		models.push_back(Backpack);

		Model Scifi("../Resources/Frog/object.obj", true);
		Scifi.setTransform(glm::mat4(1.0f), glm::vec3(-2.0f, 0.1f, 0.0f), glm::vec3(0.f, 1.f, 0.f), 0.f, glm::vec3(3.f, 3.f, 3.f));
		models.push_back(Scifi);

		Model Floor("../Resources/Floor/object.obj", true);
		Floor.setTransform(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.f, 1.f, 0.f), 0.f, glm::vec3(10.f, 0.001f, 10.f));
		statics.push_back(Floor);

		Floor.setTransform(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, -10.0f), glm::vec3(1.f, 0.f, 0.f), 90.f, glm::vec3(10.f, 0.001f, 10.f));
		statics.push_back(Floor);
}

void ObjectViewer::initLights()
{
	DirectionalLight tempDirectionalLight;
	tempDirectionalLight.color = glm::vec3(0.3, 0.3, 0.3);
	tempDirectionalLight.direction = glm::vec3(-0.5f, -1.0f, -0.5f);
	tempDirectionalLight.ambient = 0.03f;
	m_directionalLights.push_back(tempDirectionalLight);

	srand(time(NULL));
	
	for (int i = 0; i < 10; i++)
	{
		PointLight tempPointLight;
		tempPointLight.color = glm::vec3(((float)std::rand() / (RAND_MAX)), ((float)std::rand() / (RAND_MAX)), ((float)std::rand() / (RAND_MAX)));
		tempPointLight.position = glm::vec3((((float)std::rand() / (RAND_MAX)) * 20) - 10, (((float)std::rand() / (RAND_MAX)) * 9) + 1, (((float)std::rand() / (RAND_MAX)) * 20) - 10);
		tempPointLight.attenuation = glm::vec3(1.0, 0.03, 0.0015);
		m_pointLights.push_back(tempPointLight);
	}

	for (int i = 0; i < 1; i++)
	{
		SpotLight tempSpotLight;
		tempSpotLight.color = glm::vec3(0.9, 0.9, 0.9);
		tempSpotLight.position = glm::vec3(0.0, 10.0, 0.0);
		tempSpotLight.direction = glm::vec3(0.0, -1.0, 0.0);
		tempSpotLight.attenuation = glm::vec3(1.0, 0.03, 0.0015);
		tempSpotLight.cutOff = glm::cos(glm::radians(12.5f));
		tempSpotLight.outerCutOff = glm::cos(glm::radians(17.5f));
		m_spotLights.push_back(tempSpotLight);
	}

	for (auto point : m_pointLights)
	{
		std::shared_ptr<Cube> Light = std::make_shared<Cube>(point.color, 16.f, 0.8f);
		Light->resetTransform();
		Light->translate(point.position);
		Light->scale(0.5f, glm::vec3(1.f));
		lights.push_back(Light);
	}

}

#include "Scene_Handling/ObjectViewer.h"
#include <math.h>
#include <stdlib.h>
#include <time.h> 

ObjectViewer::ObjectViewer(GLFWwindow* window, std::shared_ptr<InputHandler> H) : Scene(window, H)
{
	// Resource Manager
	resourceManager = std::make_shared<ResourceManager>();
	resources = resourceManager->getResourcesPtr();

	m_camera = std::make_shared<FirstPersonCamera>();
	m_camera->attachHandler(m_window, m_handler);

	// ImGui
	m_gui = std::make_shared<Gui>(m_window);
	m_gui->bindResourcesPtr(resourceManager, resourceManager->getResourcesPtr());

	m_ColorFBO.initAttachments();
	m_ColorFBO.attachLightingShader(resources->m_shaders["LightingShader"]);
	m_ColorFBO.attachColorShader(resources->m_shaders["PPShader"]);
}

ObjectViewer::~ObjectViewer()
{

}

void ObjectViewer::update(float dt)
{
	
	m_ColorFBO.bindDefault();
	m_ColorFBO.clearScreen();

	if (m_camera->isTorch)
	{
		resources->m_torch.position = m_camera->getPosition();
		resources->m_torch.direction = m_camera->getFront();
	}

	processInput(dt);

	this->render(dt);

	m_gui->newGuiFrame();
	m_gui->drawGui();
}

void  ObjectViewer::processInput(float dt)
{

	//if (m_handler->isKeyPressed(GLFW_KEY_ESCAPE)) { glfwSetWindowShouldClose(m_window, GLFW_TRUE); }  // ESC to close
	if (m_handler->isKeyPressed(GLFW_KEY_LEFT_ALT)) {  // TAB To Enable IMGui  
		m_gui->isActive = true; // gui active if Tab pressed
		m_handler->gui = true;
	}
	if (m_handler->isKeyPressed(GLFW_KEY_TAB)) {  // ALT to Disable IMGui  
		m_gui->isActive = false; // gui disabled if Alt pressed
		m_handler->gui = false;
	}
	if (!m_gui->isActive) m_camera->update(dt); // only update if GUI not active
}

void ObjectViewer::render(float dt)
{
	if (resources->eWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	// Terrain Geometry Pass
	resources->m_shaders["TerrainShader"]->use();
	m_ColorFBO.bindGBuffer();
	m_ColorFBO.clearScreen();

	// Geometry Rendering
	this->setMatrixUniforms(resources->m_shaders["TerrainShader"]);
	this->renderTerrain(resources->m_shaders["TerrainShader"], dt);


	// Core Asset Rendering Pass
	resources->m_shaders["GeometryShader"]->use();

	// Geometry Rendering
	this->setMatrixUniforms(resources->m_shaders["GeometryShader"]);
	this->renderGeometry(resources->m_shaders["GeometryShader"], dt);

	glDisable(GL_CULL_FACE);
	
	if (resources->eDirectionalSM)
	{
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		resources->m_shaders["DShadowMapShader"]->use();
		m_ColorFBO.bindDSMFBO();
		m_ColorFBO.clearDepthBuffer();

		this->setSMUniforms(resources->m_shaders["DShadowMapShader"]);
		this->renderGeometry(resources->m_shaders["DShadowMapShader"], dt);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisable(GL_DEPTH_TEST);

	// Lighting Pass
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	m_ColorFBO.bindFBO();
	resources->m_shaders["LightingShader"]->use();
	m_ColorFBO.clearBuffer();
	this->setMatrixUniforms(resources->m_shaders["LightingShader"]);
	if (resources->eDirectionalSM)
		this->setSMUniforms(resources->m_shaders["LightingShader"]);
	this->setLightingUniforms(resources->m_shaders["LightingShader"]);
	m_ColorFBO.calcLighting();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//m_ColorFBO.bindFBO();
	// Lighting Rendering as Cubes
	resources->m_shaders["LightShader"]->use();
	this->setMatrixUniforms(resources->m_shaders["LightShader"]);
	this->renderLighting(resources->m_shaders["LightShader"], dt);

	if (resources->eBloom)
		m_ColorFBO.drawSampledBloom();

	m_ColorFBO.bindDefault();
	resources->m_shaders["PPShader"]->use();
	m_ColorFBO.clearBuffer();
	this->setPPShaderUniforms(m_ColorFBO.getActiveShader());
	m_ColorFBO.drawFrame();
}

void ObjectViewer::renderGeometry(std::shared_ptr<Shader> Shader, float dt)
{

	Shader->use();
	for (auto& model : resources->m_models)
	{
		Shader->setMat4("model", model.getTransform());
		model.Draw(Shader);
	}

}

void ObjectViewer::renderTerrain(std::shared_ptr<Shader> Shader, float dt)
{

	Shader->use();
	Shader->setMat4("model", resources->m_terrain->getTransform());
	resources->m_terrain->Draw(Shader);

}

void ObjectViewer::renderLighting(std::shared_ptr<Shader> Shader, float dt)
{
	Shader->use();

	Shader->setBool("eLightsVisible", resources->eLightsVisible);

	if (!resources->eLightsVisible)
	{
		glEnable(GL_BLEND);
	}

	for (auto& Light : resources->m_sceneObjects)
	{
		glm::mat4 temp = Light->getTransform();
		Light->setMaterialValues(Shader);
		Light->render();
	}

	if (!resources->eLightsVisible)
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
	glm::vec3 lightPos = glm::vec3((5, 25, 5));
	glm::vec3 lightDir = resources->m_directionalLight.direction;

	float left = -20.0f;
	float right = 20.0f;
	float bottom = -20.0f;
	float top = 20.0f;
	float near_plane = 0.1f;
	float far_plane = 50.f;

	glm::mat4 lightProjection = glm::ortho(left, right, bottom, top, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightDir, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;
	Shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
}

void ObjectViewer::setLightingUniforms(std::shared_ptr<Shader> Shader)
{
	Shader->setBool("eDirectional", resources->eDirectional);
	Shader->setBool("ePointLight", resources->ePointLight);
	Shader->setBool("eSpotLight", resources->eSpotLight);
	Shader->setBool("eRimming", resources->eRimming);
	Shader->setBool("eDirectionalSM", resources->eDirectionalSM);

	Shader->setVec3("dLight.color", resources->m_directionalLight.color);
	Shader->setVec3("dLight.direction", resources->m_directionalLight.direction);
	
	std::string name;

	for (int i = 0; i < resources->m_pointLights.size(); i++)
	{
		name = std::string("pLight.color[" + std::to_string(i) + "]");
		Shader->setVec3(name, resources->m_pointLights[i].color);
		name = std::string("pLight.position[" + std::to_string(i) + "]");
		Shader->setVec3(name, resources->m_pointLights[i].position);
		name = std::string("pLight.attenuation[" + std::to_string(i) + "]");
		Shader->setVec3(name, resources->m_pointLights[i].attenuation);
	}

	Shader->setFloat("pLight.numLights", resources->m_pointLights.size());


	name = std::string("sLight.color[0]");
	Shader->setVec3(name, resources->m_torch.color);
	name = std::string("sLight.position[0]");
	Shader->setVec3(name, resources->m_torch.position);
	name = std::string("sLight.direction[0]");
	Shader->setVec3(name, resources->m_torch.direction);
	name = std::string("sLight.attenuation[0]");
	Shader->setVec3(name, resources->m_torch.attenuation);
	name = std::string("sLight.cutOff[0]");
	Shader->setFloat(name, resources->m_torch.cutOff);
	name = std::string("sLight.outerCutOff[0]");
	Shader->setFloat(name, resources->m_torch.outerCutOff);


	for (int i = 1; i < resources->m_spotLights.size()+1; i++)
	{
		name = std::string("sLight.color["+ std::to_string(i) + "]");
		Shader->setVec3(name, resources->m_spotLights[i].color);
		name = std::string("sLight.position[" + std::to_string(i) + "]");
		Shader->setVec3(name, resources->m_spotLights[i].position);
		name = std::string("sLight.direction[" + std::to_string(i) + "]");
		Shader->setVec3(name, resources->m_spotLights[i].direction);
		name = std::string("sLight.attenuation[" + std::to_string(i) + "]");
		Shader->setVec3(name, resources->m_spotLights[i].attenuation);
		name = std::string("sLight.cutOff[" + std::to_string(i) + "]");
		Shader->setFloat(name, resources->m_spotLights[i].cutOff);
		name = std::string("sLight.outerCutOff[" + std::to_string(i) + "]");
		Shader->setFloat(name, resources->m_spotLights[i].outerCutOff);
	}

	//Shader->setFloat("sLight.enabled", m_camera->isTorch);
	Shader->setFloat("sLight.numLights", resources->m_spotLights.size()+1);
}

void ObjectViewer::setPPShaderUniforms(std::shared_ptr<Shader> Shader)
{
	Shader->setBool("eBloom", resources->eBloom);
	Shader->setBool("eTone", resources->eTone);
	Shader->setBool("eInverse", resources->eInverse);
	Shader->setBool("eGrayscale", resources->eGrayscale);
	Shader->setBool("eGamma", resources->eGamma);
}

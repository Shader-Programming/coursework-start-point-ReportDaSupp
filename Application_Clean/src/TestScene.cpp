#include "..\include\TestScene.h"

TestScene::TestScene(GLFWwindow* window, std::shared_ptr<InputHandler> H): Scene(window, H){
	// Shaders
	m_floorShader = std::make_shared<Shader>("..\\shaders\\floorVert.glsl", "..\\shaders\\floorFrag.glsl");

	// Camera & Input
	m_camera = std::make_shared<FirstPersonCamera>(glm::vec3(0,20,0));   
	m_camera->attachHandler(window, H);

	//imGui
	m_gui = std::make_shared<Gui>(m_window);

	//Terrain / Plane
	m_terrain = std::make_shared<Terrain>();

	//SkyBox
	m_skyBox = std::make_shared<SkyBox>();
}


TestScene::~TestScene()
{
	
	
}

void TestScene::update(float dt)
{	
	processInput(dt);
	render();		
	m_gui->newGuiFrame();
	m_gui->drawGui();
}


void TestScene::processInput(float dt)
{
	
	if (m_handler->isKeyPressed(GLFW_KEY_ESCAPE)) { glfwSetWindowShouldClose(m_window, GLFW_TRUE); }  // ESC to close
	if (m_handler->isKeyPressed(GLFW_KEY_TAB)) {  
		m_gui->isActive = false; 
		m_handler->gui = false;
	}
	if (m_handler->isKeyPressed(GLFW_KEY_LEFT_ALT)) {  
		m_gui->isActive = true; 
		m_handler->gui = true;
	}
	if (!m_gui->isActive) {
		m_camera->update(dt); // only update if GUI not active
	}
	guiVals = m_gui->getVals(); // get the gui values
}


void TestScene::render()
{	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	m_skyBox->renderSkyBox(glm::mat4(m_camera->getProjectionMatrix() * m_camera->getViewMatrix()));
	
	//floor
	// Scene Data - Lights, Camera
	m_floorShader->use();
	m_floorShader->setMat4("projection", m_camera->getProjectionMatrix());
	m_floorShader->setMat4("view", m_camera->getViewMatrix());
	m_floorShader->setMat4("model", glm::mat4(1.0f));
	m_floorShader->setVec3("viewPos", m_camera->getPosition());
	// guiVals
	m_floorShader->setVec3("floorCol", guiVals.floorCol);
	m_floorShader->setVec3("lightDirection", guiVals.lightDir);
	m_floorShader->setVec3("lightColor", guiVals.lightCol);

	//draw
	glBindVertexArray(m_terrain->getVAO());
	if (guiVals.eWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_terrain->getSize());
}



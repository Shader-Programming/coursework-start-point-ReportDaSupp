#include "..\include\TestScene.h"

TestScene::TestScene(GLFWwindow* window, std::shared_ptr<InputHandler> H): Scene(window, H){
		// Shaders
	m_floorShader = std::make_shared<Shader>("..\\shaders\\floorVert.glsl", "..\\shaders\\floorFrag.glsl");
		// Camera & Input
		m_camera = std::make_shared<FirstPersonCamera>(glm::vec3(0,20,0));   
		m_camera->attachHandler(window, H);
		//imGui
		m_gui = std::make_shared<Gui>(m_window);

		//Terrain/ Plane
		m_terrain = std::make_shared<Terrain>();
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
	if (m_handler->isKeyPressed(GLFW_KEY_TAB)) {  // TAB To use IMGui  
		m_gui->isActive = !m_gui->isActive; // gui active if tab pressed
		m_handler->gui = !m_handler->gui;
	}
	else {
		m_camera->update(dt); // only update if GUI not active
	}
	guiVals = m_gui->getVals(); // get the gui values
}


void TestScene::render()
{
	//floor
	// Scene Data - Lights, Camera
	m_floorShader->use();
	m_floorShader->setMat4("projection", m_camera->getProjectionMatrix());
	m_floorShader->setMat4("view", m_camera->getViewMatrix());
	m_floorShader->setMat4("model", glm::mat4(1.0f));
	m_floorShader->setVec3("viewPos", m_camera->getPosition());
	//
	m_floorShader->setVec3("floorCol", glm::vec3(0.2, 0.9, 0.3));
	m_floorShader->setVec3("lightDirection", guiVals.lightDir);


	//draw
	glBindVertexArray(m_terrain->getVAO());
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, m_terrain->getSize());
}



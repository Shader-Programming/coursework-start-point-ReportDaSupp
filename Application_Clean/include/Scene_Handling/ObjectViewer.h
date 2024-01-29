#pragma once

#include "Scene.h"
#include "Shaders/Shader.h"
#include "Scene_Data/Textures/TextureManager.h"
#include "Scene_Data/Framebuffers/Framebuffer.h"
#include "Inputs&Cameras/Gui.h"

/*
Child Class Of Scene (Used For Rendering Single Objects)
*/
class ObjectViewer : protected Scene {

public:
	ObjectViewer(GLFWwindow* window, std::shared_ptr<InputHandler> H);
	~ObjectViewer();
	void update(float dt) override;

private:
	void processInput(float dt);
	void render(float dt);
	void renderGeometry(std::shared_ptr<Shader> Shader, float dt);
	void renderLighting(std::shared_ptr<Shader> Shader, float dt);
	void setMatrixUniforms(std::shared_ptr<Shader> Shader);
	void setSMUniforms(std::shared_ptr<Shader> Shader);
	void setLightingUniforms(std::shared_ptr<Shader> Shader);
	void setPPShaderUniforms(std::shared_ptr<Shader> Shader);

	glm::mat4 m_view, m_projection;

	std::shared_ptr<Gui> m_gui;
	std::shared_ptr<ResourceManager> resourceManager;
	std::shared_ptr<Resources> resources;

};
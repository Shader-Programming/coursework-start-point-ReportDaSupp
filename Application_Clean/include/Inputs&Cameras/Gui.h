#pragma once
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw.h"
#include "IMGUI/imgui_impl_opengl3.h"
#include "ImGui/imgui_stdlib.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/common.hpp>

struct guiVars
{
	std::string localAssetPath = "/Frog/object.obj";
	glm::vec3 localAssetPosition = {0.f, 0.f, 0.f};
	glm::vec3 localAssetRotation = {1.f, 1.f, 1.f};
	float localRotateAngle = 0.f;
	glm::vec3 localAssetScale = { 1.f, 1.f, 1.f };
	bool texFlipped = true;
	bool isStatic = true;
};

class Gui {

public:

	Gui(GLFWwindow* window);
	~Gui();
	void newGuiFrame();
	void drawGui();
	bool isActive = false;


private:
	GLFWwindow* m_window;
	guiVars guiVariables;

	ImVec4 TitleColor = { 1.f, 0.5f, 0.f, 1.f };
	ImVec4 SubTitleColor = { 0.3f, 0.3f, 1.f, 1.f };
};

#pragma once
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw.h"
#include "IMGUI/imgui_impl_opengl3.h"
#include "ImGui/imgui_stdlib.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/common.hpp>

#ifndef GUI_DATA_H
#define GUI_DATA_H

struct guiVars {
	bool isWireframe = false;
	bool isPBR = false;
	bool isSphere = true;
	bool isActive = false;
	bool isMoving = true;
};

extern guiVars g_guiData;

#endif // GUI_DATA_H

class Gui {

public:

	Gui(GLFWwindow* window);
	~Gui();
	void newGuiFrame();
	void drawGui();


private:
	GLFWwindow* m_window;

	ImVec4 TitleColor = { 1.f, 0.5f, 0.f, 1.f };
	ImVec4 SubTitleColor = { 0.3f, 0.3f, 1.f, 1.f };
};
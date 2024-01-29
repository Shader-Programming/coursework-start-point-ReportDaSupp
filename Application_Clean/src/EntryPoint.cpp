#include "Windows/WindowManager.h"
#include "Globals/Properties.h"
#include "Scene_Handling/ObjectViewer.h"

/*
Entry point for App
Creates a window with an input handler
Creates instances of a scene
Runs game loop
*/

void main() {

	WindowManager windowMan("IMAT3907 Assessed Program", SCR_WIDTH, SCR_HEIGHT, true);  // name, width, height, vSync,
	ObjectViewer scene(windowMan.getWindow(), windowMan.getHandler()); // pass GLFWwindow and Input Handler to scene

	while (!glfwWindowShouldClose(windowMan.getWindow())) {


		windowMan.startFrame();
		scene.update(windowMan.getDeltaTime());
		windowMan.endFrame();

	}
	windowMan.shutDown();

}


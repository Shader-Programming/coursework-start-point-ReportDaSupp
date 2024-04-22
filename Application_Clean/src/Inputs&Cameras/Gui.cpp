#include "Inputs&Cameras/Gui.h"
#include "Globals/Properties.h"


guiVars g_guiData;

Gui::Gui(GLFWwindow* window) : m_window(window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init();
    ImGui::StyleColorsDark();

}

Gui::~Gui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}

void Gui::newGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    if(g_guiData.isActive) glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    else glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
}

void Gui::drawGui()
{
    float ms = 1000.0f / ImGui::GetIO().Framerate;

    if (ImGui::Begin("Test"))
    {
        ImGui::Checkbox("Wireframe", &g_guiData.isWireframe);
        ImGui::Checkbox("Sphere", &g_guiData.isSphere);
        ImGui::Checkbox("PBR", &g_guiData.isPBR);
        ImGui::Checkbox("Moving", &g_guiData.isMoving);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

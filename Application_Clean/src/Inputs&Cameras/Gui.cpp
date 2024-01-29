#include "Inputs&Cameras/Gui.h"


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
    if(isActive) glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    else glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
}

void Gui::drawGui()
{
    float ms = 1000.0f / ImGui::GetIO().Framerate; ;  //milisecconds
    if (ImGui::BeginMainMenuBar())
    {
        ImGui::TextColored(TitleColor, "Advanced Shader Programming ");
        
        ImGui::Separator();

        if (ImGui::BeginMenu("Assets"))
        {
            if (isActive)
            {
                ImGui::TextColored(SubTitleColor, "Asset Loading");
                ImGui::InputText("Asset Filepath", &guiVariables.localAssetPath);
                ImGui::InputFloat3("Position", &guiVariables.localAssetPosition.x);
                ImGui::InputFloat3("Rotation", &guiVariables.localAssetRotation.x);
                ImGui::InputFloat("Rotation Angle", &guiVariables.localAssetRotation.x);
                ImGui::InputFloat3("Scale", &guiVariables.localAssetScale.x);
                ImGui::Checkbox("is Flipped Textures", &guiVariables.texFlipped);
                ImGui::Checkbox("is Asset Static", &guiVariables.isStatic);
                ImGui::Text("");
                if (ImGui::Button("Load Asset"))
                {
                    resourceManager->initAssimpModel(("../Resources/Models" + guiVariables.localAssetPath).c_str(), guiVariables.texFlipped, guiVariables.isStatic, guiVariables.localAssetPosition, guiVariables.localAssetRotation, guiVariables.localRotateAngle, guiVariables.localAssetScale);
                }
                
            ImGui::EndMenu();
            }
        }

        if (ImGui::BeginMenu("Settings"))
        {
            if (isActive)
            {
                ImGui::TextColored(SubTitleColor, "Lighting");
                ImGui::Checkbox("Directional", &resources->eDirectional);
                ImGui::Checkbox("Point", &resources->ePointLight);
                ImGui::Checkbox("Spot", &resources->eSpotLight);
                ImGui::Checkbox("Rim", &resources->eRimming);
                ImGui::Checkbox("Render Lights", &resources->eLightsVisible);
                ImGui::Text("");
                ImGui::TextColored(SubTitleColor, "Post-Proccessing");
                ImGui::Checkbox("Shadows", &resources->eDirectionalSM);
                ImGui::Checkbox("Bloom", &resources->eBloom);
                ImGui::Checkbox("Tone", &resources->eTone);
                ImGui::Checkbox("Gamma", &resources->eGamma);
                ImGui::Checkbox("Grayscale", &resources->eGrayscale);
                ImGui::Checkbox("Inverse", &resources->eInverse);

                ImGui::EndMenu();
            }
        }

        ImGui::Separator();

        if (ImGui::SmallButton("Close"))
        {
            glfwSetWindowShouldClose(m_window, GLFW_TRUE);
        }

    ImGui::EndMainMenuBar();
    }
    //ImGui::Text("FPS %.3f ms/frame (%.1f FPS)", ms, ImGui::GetIO().Framerate);  // display FPS and ms
    //ImGui::ShowDemoWindow();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

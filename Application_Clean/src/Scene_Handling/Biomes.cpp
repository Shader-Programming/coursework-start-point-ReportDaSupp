#include "Scene_Handling/Biomes.h"

void Biomes::initializeBiomes() {
    earth = std::make_shared<celestialBody>(true, true, 1);
    m_camera = std::make_unique<FirstPersonCamera>();
    m_camera->attachHandler(m_window, m_handler);
    ImGuiInterface = std::make_shared<Gui>(m_window);
}

void Biomes::update(float dt) {
    
    
    if (g_guiData.isMoving)
        timeElapsed += dt;

    updateCamera(dt);

    float angle = (timeElapsed) * moonOrbitSpeed;
    float x = moonOrbitRadius * sin(angle);
    float z = moonOrbitRadius * cos(angle);

    glm::mat4 earthModel(1.0f);
    earthModel = glm::translate(earthModel, glm::vec3(0, 0, 0));
    earthModel = glm::rotate(earthModel, (float)glm::radians(timeElapsed), glm::vec3(0, 1, 0));
    earthModel = glm::scale(earthModel, glm::vec3(100, 100, 100));
    

    glm::mat4 moonModel(1.0f);
    
    moonModel = glm::translate(moonModel, glm::vec3(x, z / 4, z));
    moonModel = glm::rotate(moonModel, (float)angle, glm::vec3(0, 1, 0));
    moonModel = glm::scale(moonModel, glm::vec3(25, 25, 25));

    if (g_guiData.isWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    earth->renderMoon(m_camera->getViewMatrix(), m_camera->getProjectionMatrix(), m_camera->getPosition(), moonModel, timeElapsed);
    earth->renderPlanet(m_camera->getViewMatrix(), m_camera->getProjectionMatrix(), m_camera->getPosition(), earthModel, timeElapsed);

    ImGuiInterface->newGuiFrame();
    ImGuiInterface->drawGui();
    
}

void Biomes::updateWeather(float dt) {
    // Weather system updates are handled here
}

void Biomes::updateCamera(float dt) {
    m_camera->update(dt);
}
#pragma once

#include "Scene.h"
#include "TerrainGenerator.h"
#include "WeatherSystem.h"
#include "BiomeRenderer.h"
#include "BiomeDistributor.h"
#include "Globals/Properties.h"

struct celestialBody
{
    celestialBody(bool water = false, bool atmosphere = false, float moons = 0)
    {
        m_biomeDistributor = std::make_unique<BiomeDistributor>("../Shaders/Scene/BiomeDistributor.glsl");
        m_terrainGenerator = std::make_unique<TerrainGenerator>("../Shaders/Scene/TerrainGenerator.glsl");
        m_biomeRenderer = std::make_unique<BiomeRenderer>(m_terrainGenerator->getTerrainVAO(), "../Shaders/Scene/BiomeRenderer.glsl", "../Shaders/Scene/PBRBiomeRenderer.glsl", "../Shaders/Scene/WaterRenderer.glsl", "../Shaders/Scene/AtmosphereRenderer.glsl", "../Shaders/Scene/MoonRenderer.glsl", m_terrainGenerator->getIndexCount());
        m_weatherSystem = std::make_unique<WeatherSystem>("../Shaders/Scene/SkyBox.glsl", "../Shaders/Scene/Clouds.glsl");

        isWater = water;
        isAtmosphere = atmosphere;
        numMoons = moons;
    }

    void renderPlanet(glm::mat4 view, glm::mat4 proj, glm::vec3 pos, glm::mat4 model, float dt)
    {
        m_weatherSystem->renderSkybox(view, proj);

        if (g_guiData.isPBR)
        {
            m_biomeRenderer->setupShaderWithMaps(m_biomeDistributor->getHeightMap(), m_biomeDistributor->getTemperatureMap(), m_biomeDistributor->getPrecipitationMap(), m_biomeDistributor->getDuDvMap(), m_biomeRenderer->getPBRPlanetShader());
            m_biomeRenderer->renderPlanet(view, proj, pos, model, dt, m_biomeRenderer->getPBRPlanetShader());
        }
        else
        {
            m_biomeRenderer->setupShaderWithMaps(m_biomeDistributor->getHeightMap(), m_biomeDistributor->getTemperatureMap(), m_biomeDistributor->getPrecipitationMap(), m_biomeDistributor->getDuDvMap(), m_biomeRenderer->getPlanetShader());
            m_biomeRenderer->renderPlanet(view, proj, pos, model, dt, m_biomeRenderer->getPlanetShader());
        }

        if (isWater) {
            m_biomeRenderer->setupShaderWithMaps(m_biomeDistributor->getHeightMap(), m_biomeDistributor->getTemperatureMap(), m_biomeDistributor->getPrecipitationMap(), m_biomeDistributor->getDuDvMap(), m_biomeRenderer->getWaterShader());
            m_weatherSystem->loadSkybox(m_biomeRenderer->getWaterShader());
            m_biomeRenderer->renderWater(view, proj, pos, model, dt);
        }

        m_weatherSystem->renderClouds(view, proj, pos);

        if (isAtmosphere) {
            m_biomeRenderer->setupShaderWithMaps(m_biomeDistributor->getHeightMap(), m_biomeDistributor->getTemperatureMap(), m_biomeDistributor->getPrecipitationMap(), m_biomeDistributor->getDuDvMap(), m_biomeRenderer->getAtmosphereShader());
            m_biomeRenderer->renderAtmosphere(view, proj, pos, model, dt);
        }
    }

    void renderMoon(glm::mat4 view, glm::mat4 proj, glm::vec3 pos, glm::mat4 model, float dt)
    {
        for (int i = 0; i < numMoons; i++)
        {
            m_biomeRenderer->setupShaderWithMaps(m_biomeDistributor->getHeightMap(), m_biomeDistributor->getTemperatureMap(), m_biomeDistributor->getPrecipitationMap(), m_biomeDistributor->getDuDvMap(), m_biomeRenderer->getMoonShader());
            m_biomeRenderer->renderMoon(view, proj, pos, model, dt);
        }
    }

    std::unique_ptr<TerrainGenerator> m_terrainGenerator;
    std::unique_ptr<WeatherSystem> m_weatherSystem;
    std::unique_ptr<BiomeRenderer> m_biomeRenderer;
    std::unique_ptr<BiomeDistributor> m_biomeDistributor;

    bool isWater = false;
    bool isAtmosphere = false;
    float numMoons = 0;

};

// Biomes Scene for handling different environments like sand dunes, forests, and oceans
class Biomes : public Scene {
public:
    Biomes(GLFWwindow* window, std::shared_ptr<InputHandler> handler) : Scene(window, handler) {
        initializeBiomes();
    }

    ~Biomes() {}

    // Update function to handle real-time updates in the scene
    void update(float dt) override;

protected:
    void initializeBiomes();
    void updateWeather(float dt);
    void updateCamera(float dt);

private:
    std::shared_ptr<celestialBody> earth;
    std::unique_ptr<FirstPersonCamera> m_camera;

    float timeElapsed = 0;

    float moonOrbitRadius = 400.f;
    float moonOrbitSpeed = 0.06f;
    bool isPBRLoaded = false;

    std::shared_ptr<Gui> ImGuiInterface;
};
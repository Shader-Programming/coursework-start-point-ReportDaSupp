#include "Scene_Handling/Biomes.h"

void Biomes::initializeBiomes() {
    m_biomeDistributor = std::make_unique<BiomeDistributor>("../Shaders/Scene/BiomeDistributor.glsl");
    m_terrainGenerator = std::make_unique<TerrainGenerator>("../Shaders/Scene/TerrainGenerator.glsl");
    m_biomeRenderer = std::make_unique<BiomeRenderer>(m_terrainGenerator->getTerrainVAO(), "../Shaders/Scene/BiomeRenderer.glsl");
    m_weatherSystem = std::make_unique<WeatherSystem>("../Shaders/Scene/SkyBox.glsl");
    m_camera = std::make_unique<FirstPersonCamera>();
    m_camera->attachHandler(m_window, m_handler);
}

void Biomes::update(float dt) {
    
    updateTerrain(dt);
    updateCamera(dt);

    m_weatherSystem->renderSkybox(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

    // Render current view of the planet
    m_biomeRenderer->setupShaderWithMaps(m_biomeDistributor->getHeightMap(), m_biomeDistributor->getTemperatureMap(), m_biomeDistributor->getPrecipitationMap());
    m_biomeRenderer->renderPlanet(m_camera->getViewMatrix(), m_camera->getProjectionMatrix(), m_terrainGenerator->getIndexCount(), m_camera->getPosition(), dt);
}

void Biomes::updateWeather(float dt) {
    // Weather system updates are handled here
}

void Biomes::updateTerrain(float dt) {
    // Check if the terrain needs updating due to environmental factors or changes in the scene
}

void Biomes::updateCamera(float dt) {
    m_camera->update(dt);
}
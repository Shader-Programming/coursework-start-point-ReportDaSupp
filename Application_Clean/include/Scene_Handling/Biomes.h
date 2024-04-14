#pragma once

#include "Scene.h"
#include "TerrainGenerator.h"
#include "WeatherSystem.h"
#include "BiomeRenderer.h"
#include "BiomeDistributor.h"

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
    void updateTerrain(float dt);
    void updateCamera(float dt);

private:
    std::unique_ptr<TerrainGenerator> m_terrainGenerator;
    std::unique_ptr<WeatherSystem> m_weatherSystem;
    std::unique_ptr<BiomeRenderer> m_biomeRenderer;
    std::unique_ptr<BiomeDistributor> m_biomeDistributor;
    std::unique_ptr<FirstPersonCamera> m_camera;
};

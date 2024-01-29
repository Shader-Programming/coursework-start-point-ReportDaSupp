#pragma once

#include "Scene.h"
#include "Shaders/Shader.h"
#include "Scene_Data/Lights/LightsBase.h"
#include "Scene_Data/Objects/Cube.h"
#include "Scene_Data/Objects/Skybox.h"
#include "Scene_Data/Objects/AssimpObj.h"
#include "Scene_Data/Textures/TextureManager.h"
#include "Scene_Data/Framebuffers/Framebuffer.h"

/*
Child Class Of Scene (Used For Rendering Single Objects)
*/
class ObjectViewer : protected Scene {

public:
	ObjectViewer(GLFWwindow* window, std::shared_ptr<InputHandler> H);
	~ObjectViewer();
	void update(float dt) override;

private:
	void render(float dt);
	void renderGeometry(std::shared_ptr<Shader> Shader, float dt);
	void renderLighting(std::shared_ptr<Shader> Shader, float dt);
	void setMatrixUniforms(std::shared_ptr<Shader> Shader);
	void setSMUniforms(std::shared_ptr<Shader> Shader);
	void setLightingUniforms(std::shared_ptr<Shader> Shader);
	void setPPShaderUniforms(std::shared_ptr<Shader> Shader);
	void initModels();
	void initLights();

	glm::mat4 m_view, m_projection;

	float lerp = 0;

	std::vector<DirectionalLight> m_directionalLights;
	std::vector<PointLight> m_pointLights;
	std::vector<SpotLight> m_spotLights;

	std::shared_ptr<Shader> m_GeometryShader;
	std::shared_ptr<Shader> m_LightShader;
	std::shared_ptr<Shader> m_LightingShader;
	std::shared_ptr<Shader> m_PPShader;
	std::shared_ptr<Shader> m_DShadowMapShader;

	std::vector<std::shared_ptr<BaseObject>> lights;

	std::vector<Model> models;
	std::vector<Model> statics;

	std::shared_ptr<BaseObject> Floor;

	std::shared_ptr<BaseObject> Skybox;

	std::shared_ptr<BaseObject> Wall1;
	std::shared_ptr<BaseObject> Wall2;

};
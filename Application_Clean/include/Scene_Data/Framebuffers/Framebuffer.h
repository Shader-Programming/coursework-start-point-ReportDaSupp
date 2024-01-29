#pragma once
#include "Scene_Data/Objects/Plane.h"
#include "Shaders/Shader.h"
#include "Globals/Properties.h"
#include <unordered_map>

struct MipData
{
	uint32_t FBO;
	uint32_t textures[5];
	glm::vec2 mipSize[5];
};

class Framebuffer
{
public:
	Framebuffer() = default;
	~Framebuffer();

	void initAttachments();

	void calcLighting();
	void drawSampledBloom();
	void drawFrame();

	inline void bindDefault() { glBindFramebuffer(GL_FRAMEBUFFER, 0); };
	inline void bindDown() { glBindFramebuffer(GL_FRAMEBUFFER, DownSample.FBO); };
	inline void bindUp() { glBindFramebuffer(GL_FRAMEBUFFER, UpSample.FBO); };
	inline void bindFBO() { glBindFramebuffer(GL_FRAMEBUFFER, m_FBO); };
	inline void bindDSMFBO() { glBindFramebuffer(GL_FRAMEBUFFER, m_DSMFBO); };
	inline void bindGBuffer() { glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer); };
	inline void clearScreen() { glClearColor(0.1f, 0.1f, 0.1f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); };
	inline void clearBuffer() { glClearColor(0.0f, 0.0f, 0.0f, 1.0f); glClear(GL_COLOR_BUFFER_BIT); };
	inline void clearDepthBuffer() { glClearColor(0.0f, 0.0f, 0.0f, 1.0f); glClear(GL_DEPTH_BUFFER_BIT); };

	inline void attachLightingShader(std::shared_ptr<Shader> Shader) { m_LightingShader = Shader; }
	inline void attachColorShader(std::shared_ptr<Shader> Shader) { m_ActiveColorShader = Shader; }

	std::shared_ptr<Shader> getActiveShader() { return m_ActiveColorShader; };


private:
	uint32_t m_GBuffer, m_FBO, m_DSMFBO;
	
	MipData DownSample;
	MipData UpSample;
	
	std::unordered_map<std::string, uint32_t> m_BufferAttachments;
	uint32_t ShadowMapTex;

	std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
	std::shared_ptr<Shader> m_LightingShader;
	std::shared_ptr<Shader> m_ActiveColorShader;
	std::shared_ptr<Shader> m_DownSampleShader;
	std::shared_ptr<Shader> m_UpSampleShader;

	std::shared_ptr<Plane> m_ScreenQuad;
	
	void initGBuffer();
	void initColorFBO();
	void initDownFBO();
	void initUpFBO();
	void initDirectionalSM();
};

extern Framebuffer m_ColorFBO;
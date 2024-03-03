#include "Scene_Data/Framebuffers/Framebuffer.h"

Framebuffer m_ColorFBO;

Framebuffer::~Framebuffer()
{
	
}

void Framebuffer::initAttachments()
{
	m_ScreenQuad = std::make_shared<Plane>();

	m_HeightMapShader = std::make_shared<Shader>("..\\Shaders\\Scene\\HeightMap.glsl");

	this->initGBuffer();
	this->initColorFBO();
	this->initDownFBO();
	this->initUpFBO();
	this->initDirectionalSM();
	this->initHeightMap();

	m_DownSampleShader = std::make_shared<Shader>("..\\Shaders\\Post Processing\\DownSampleShader.vert", "..\\Shaders\\Post Processing\\DownSampleShader.frag");
	m_UpSampleShader = std::make_shared<Shader>("..\\Shaders\\Post Processing\\UpSampleShader.vert", "..\\Shaders\\Post Processing\\UpSampleShader.frag");

	
}

void Framebuffer::calcLighting()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO); // write to default framebuffer
	glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	m_LightingShader->use();
	m_ScreenQuad->setMaterialValues(m_LightingShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_BufferAttachments["Position"]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_BufferAttachments["NormalMap"]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_BufferAttachments["AlbedoSpec"]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_BufferAttachments["Normal"]);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, m_BufferAttachments["TBN1"]);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, m_BufferAttachments["TBN2"]);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, m_BufferAttachments["TBN3"]);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, ShadowMapTex);

	m_LightingShader->setInt("gPosition", 0);
	m_LightingShader->setInt("gNormalMap", 1);
	m_LightingShader->setInt("gAlbedoSpec", 2);
	m_LightingShader->setInt("gNormal", 3);
	m_LightingShader->setInt("gTBN1", 4);
	m_LightingShader->setInt("gTBN2", 5);
	m_LightingShader->setInt("gTBN3", 6);

	m_LightingShader->setInt("ShadowMap", 7);

	m_ScreenQuad->render();

	glActiveTexture(GL_TEXTURE0);

}

void Framebuffer::drawSampledBloom()
{
	this->bindDown();
	m_DownSampleShader->use();

	for (int i = 0; i < 5; i++)
	{
		if (i == 0)
		{
			m_DownSampleShader->setVec2("Resolution", { SCR_WIDTH, SCR_HEIGHT });
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_BufferAttachments["BrightColorAttachment"]);
			m_DownSampleShader->setInt("DownSampleTexture", 0);
		}
		else
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, DownSample.textures[i-1]);
			m_DownSampleShader->setInt("DownSampleTexture", 0);
			m_DownSampleShader->setVec2("Resolution", { (int)DownSample.mipSize[i-1].x, (int)DownSample.mipSize[i-1].y });
		}

		glViewport(0, 0, DownSample.mipSize[i].x, DownSample.mipSize[i].y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, DownSample.textures[i], 0);

		glClear(GL_COLOR_BUFFER_BIT);

		m_ScreenQuad->setMaterialValues(m_DownSampleShader);
		m_ScreenQuad->render();
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	this->bindUp();
	m_UpSampleShader->use();

	for (int i = 0; i < 5; i++)
	{

		if (i == 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, DownSample.textures[4]);
			m_UpSampleShader->setInt("UpSampleTexture", 0);
			m_UpSampleShader->setFloat("filterRadius", 1 / DownSample.mipSize[4].y);
		}
		else
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, UpSample.textures[i-1]);
			m_UpSampleShader->setInt("UpSampleTexture", 0);
			m_UpSampleShader->setFloat("filterRadius", 1.0 / UpSample.mipSize[i-1].y);
		}

		glViewport(0, 0, UpSample.mipSize[i].x, UpSample.mipSize[i].y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, UpSample.textures[i], 0);

		glClear(GL_COLOR_BUFFER_BIT);

		m_ScreenQuad->setMaterialValues(m_UpSampleShader);
		m_ScreenQuad->render();
	}

	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);

}

void Framebuffer::drawFrame()
{
	m_ActiveColorShader->use();
	m_ScreenQuad->setMaterialValues(m_ActiveColorShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_BufferAttachments["ColorAttachment"]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, UpSample.textures[4]);

	m_ActiveColorShader->setInt("ColorAttachment", 0);
	m_ActiveColorShader->setInt("BloomAttachment", 1);


	m_ScreenQuad->render();
}

void Framebuffer::initHeightMap()
{
	glGenTextures(1, &HeightMapTex);
	glBindTexture(GL_TEXTURE_2D, HeightMapTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 2048, 2048, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &DuDvMapTex);
	glBindTexture(GL_TEXTURE_2D, DuDvMapTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 2048, 2048, 0, GL_RED, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Framebuffer::initGBuffer()
{
	glGenFramebuffers(1, &m_GBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer);

	uint32_t gPosition;

	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	uint32_t gNormalMap;

	// normal color buffer
	glGenTextures(1, &gNormalMap);
	glBindTexture(GL_TEXTURE_2D, gNormalMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalMap, 0);

	uint32_t gAlbedoSpec;

	// color + specular color buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	uint32_t gNormal;

	// normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gNormal, 0);

	uint32_t gTBN1;

	// tbn1 data buffer
	glGenTextures(1, &gTBN1);
	glBindTexture(GL_TEXTURE_2D, gTBN1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gTBN1, 0);

	uint32_t gTBN2;

	// tbn2 data buffer
	glGenTextures(1, &gTBN2);
	glBindTexture(GL_TEXTURE_2D, gTBN2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, gTBN2, 0);

	uint32_t gTBN3;

	// tbn3 data buffer
	glGenTextures(1, &gTBN3);
	glBindTexture(GL_TEXTURE_2D, gTBN3);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, gTBN3, 0);


	m_BufferAttachments["Position"] = gPosition;
	m_BufferAttachments["NormalMap"] = gNormalMap;
	m_BufferAttachments["AlbedoSpec"] = gAlbedoSpec;
	m_BufferAttachments["Normal"] = gNormal;
	m_BufferAttachments["TBN1"] = gTBN1;
	m_BufferAttachments["TBN2"] = gTBN2;
	m_BufferAttachments["TBN3"] = gTBN3;

	unsigned int GeometryAttachments[7] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6};
	glDrawBuffers(7, GeometryAttachments);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
}

void Framebuffer::initColorFBO()
{
	// Color FrameBuffer Pre-Processing
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	uint32_t gColorAttachment, gBrightColorAttachment;

	glGenTextures(1, &gColorAttachment);
	glBindTexture(GL_TEXTURE_2D, gColorAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gColorAttachment, 0);

	glGenTextures(1, &gBrightColorAttachment);
	glBindTexture(GL_TEXTURE_2D, gBrightColorAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBrightColorAttachment, 0);

	m_BufferAttachments["ColorAttachment"] = gColorAttachment;
	m_BufferAttachments["BrightColorAttachment"] = gBrightColorAttachment;

	unsigned int ColorAttachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, ColorAttachments);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
}

void Framebuffer::initDownFBO()
{
	// Bloom FrameBuffer DownSample Via Mip Chain
	glGenFramebuffers(1, &DownSample.FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, DownSample.FBO);

	for (int i = 0; i < 5; i++)
	{
		DownSample.mipSize[i] = glm::vec2((float)SCR_WIDTH * (float)pow(0.5f, (float)(i + 1)), (float)SCR_HEIGHT * (float)pow(0.5f, (float)(i + 1)));

		glGenTextures(1, &DownSample.textures[i]);
		glBindTexture(GL_TEXTURE_2D, DownSample.textures[i]);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, DownSample.mipSize[i].x, DownSample.mipSize[i].y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, DownSample.textures[0], 0);
}

void Framebuffer::initUpFBO()
{
	// Bloom FrameBuffer UpSample Via Mip Chain
	glGenFramebuffers(1, &UpSample.FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, UpSample.FBO);

	for (int i = 0; i < 5; i++)
	{
		UpSample.mipSize[i] = glm::vec2((float)SCR_WIDTH * (float)pow(0.5f, (float)(4 - i)), (float)SCR_HEIGHT * (float)pow(0.5f, (float)(4 - i)));

		glGenTextures(1, &UpSample.textures[i]);
		glBindTexture(GL_TEXTURE_2D, UpSample.textures[i]);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, UpSample.mipSize[i].x, UpSample.mipSize[i].y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, UpSample.textures[0], 0);
}

void Framebuffer::initDirectionalSM()
{
	glGenFramebuffers(1, &m_DSMFBO);

	glGenTextures(1, &ShadowMapTex);
	glBindTexture(GL_TEXTURE_2D, ShadowMapTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, m_DSMFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ShadowMapTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::runHeightMap(glm::vec3 viewPos, float et)
{
	m_HeightMapShader->use();
	m_HeightMapShader->setFloat("ellapsedTime", et);
	m_HeightMapShader->setVec3("viewPos", viewPos);
	glBindImageTexture(0, HeightMapTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(1, DuDvMapTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glDispatchCompute((GLuint)ceil(2048 / 32.0f), (GLuint)ceil(2048 / 32.0f), 1);

	// Wait for compute shader to complete before using the texture
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

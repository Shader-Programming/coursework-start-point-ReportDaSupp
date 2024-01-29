#pragma once
#include "Scene_Data/Textures/stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shaders/Shader.h"
#include <memory>
#include <utility>
#include <vector>

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

class TextureManager
{
public:
	TextureManager() {
		Textures.clear();
	};
	~TextureManager() { Textures.clear(); }

	static uint32_t loadTexture(std::string path);

	std::shared_ptr<Texture> assignTexture(std::string filepath, std::string texType);
	std::shared_ptr<Texture> assignFrameTexture(unsigned int texID);
	std::shared_ptr<Texture> assignFrameCubeTexture(unsigned int texID);

	void setTexUniform(std::shared_ptr<Shader> Shader, std::string texName, std::shared_ptr<Texture> texture);
	
protected:
	std::vector<std::shared_ptr<Texture>> Textures;
};

extern TextureManager g_TextureManager;
#include "Scene_Data/Textures/TextureManager.h"

TextureManager g_TextureManager;

uint32_t TextureManager::loadTexture(std::string path)
{
	uint32_t textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);

	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, format, GL_UNSIGNED_BYTE, data); // whats this?
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR); // S = x axis 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR); // T == y axis, R if 3D
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
	}
	else
	{
		stbi_image_free(data);
	}
	return textureID;
}

std::shared_ptr<Texture> TextureManager::assignTexture(std::string filepath, std::string texType)
{
	for (int i = 0; i < Textures.size(); i++)
	{
		if (std::strcmp(Textures[i]->path.c_str(), filepath.c_str()) == 0)
		{
			return Textures[i];
		}
	}

	std::shared_ptr<Texture> temp = std::make_shared<Texture>();
	temp->path = filepath;
	temp->id = loadTexture(filepath);
	temp->type = texType;

	glActiveTexture(GL_TEXTURE0 + temp->id);
	glBindTexture(GL_TEXTURE_2D, temp->id);
	glActiveTexture(NULL);

	Textures.push_back(temp);
	return temp;
}

std::shared_ptr<Texture> TextureManager::assignFrameTexture(unsigned int texID)
{
	std::shared_ptr<Texture> temp = std::make_shared<Texture>();
	temp->path = "";
	temp->id = texID;
	temp->type = "FrameTexture";

	glActiveTexture(GL_TEXTURE0 + temp->id);
	glBindTexture(GL_TEXTURE_2D, temp->id);

	return temp;
}

std::shared_ptr<Texture> TextureManager::assignFrameCubeTexture(unsigned int texID)
{
	std::shared_ptr<Texture> temp = std::make_shared<Texture>();
	temp->path = "";
	temp->id = texID;
	temp->type = "FrameCubeTexture";

	glActiveTexture(GL_TEXTURE0 + temp->id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, temp->id);

	return temp;
}

void TextureManager::setTexUniform(std::shared_ptr<Shader> Shader, std::string texName, std::shared_ptr<Texture> texture)
{
	Shader->setInt(texName, texture->id);
}

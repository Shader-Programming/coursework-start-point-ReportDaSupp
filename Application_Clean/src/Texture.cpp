
#include "Texture.h"
#include "glad/glad.h"
#include <iostream>



Texture::Texture(const char* filePath) {

	int width, height, nrComponents;

	unsigned char* data = stbi_load(filePath, &width, &height, &nrComponents, 0);

	if (data) createTexture(width, height, nrComponents, data);
	else std::cout << "Texture failed to load at path: " << filePath << std::endl;
	stbi_image_free(data);

}

Texture::~Texture() {
	glDeleteTextures(1, &m_id);
}

void Texture::createTexture(unsigned int width, unsigned int height, unsigned int channels, unsigned char* data) {


	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);

	GLenum format;
	if (channels == 1)
		format = GL_RED;
	else if (channels == 3)
		format = GL_RGB;
	else if (channels == 4)
		format = GL_RGBA32F;

	glActiveTexture(GL_TEXTURE0);//activate texture unit
	glBindTexture(GL_TEXTURE_2D, m_id);// bind texture object
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data); // send data
	std::cout << format << " " << channels << std::endl;

	// set texture object parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);



}





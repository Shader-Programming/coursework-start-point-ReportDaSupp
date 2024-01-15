#pragma once
#include "stb_image.h"

class Texture {
public:
	Texture(const char* filePath);
	~Texture();
	inline unsigned int getID() { return m_id; }


private:
	unsigned int m_id;
	void createTexture(unsigned int width, unsigned int height, unsigned int channels, unsigned char* data);


};
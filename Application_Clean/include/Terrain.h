#pragma once


#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class Terrain
{
public:
	Terrain(int widthIn, int heightIn, int stepSizeIn);
	Terrain();
	void makeVAO();
	unsigned int getVAO() { return m_VAO; }
	int getSize();


private:
	std::vector<float> m_vertices;
	unsigned int m_VAO;
	int width;
	int height;
	int stepSize;
	void makeVertices();
	void makeVertex(int x, int z);
	std::vector<float> getVertices();
};
#pragma once
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "Texture.h"
#include "Shader.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	glm::vec3 tangent;
};

class Mesh {

public:
	Mesh() {};

	inline void addVertex(Vertex v) { m_vertices.push_back(v); }
	inline void addIndex(unsigned int index) { m_indices.push_back(index); }
	//inline unsigned int getID() { return m_id; }
	void createBuffers();
	void drawMesh();





private:

	//unsigned int m_id;
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;
	unsigned int m_VAO;

};


#include "Terrain.h"


//Terrain constructors
Terrain::Terrain(int widthIn, int heightIn, int stepSizeIn)
{
	width = widthIn;
	height = heightIn;
	stepSize = stepSizeIn;
	makeVertices();
	makeVAO();
}

Terrain::Terrain() {
	width = 50;
	height = 50;
	stepSize = 10;
	makeVertices();
	makeVAO();

}

void Terrain::makeVAO() {

	unsigned int VBO;
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, (m_vertices.size() * sizeof(GLfloat)), m_vertices.data(), GL_STATIC_DRAW);

	//xyz
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//texture
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int Terrain::getSize() {
	return m_vertices.size();
}


std::vector<float> Terrain::getVertices() {
	return m_vertices;
}


void Terrain::makeVertices() {
	/* triangle a b c
		   b
		   | \
		   a _ c


		 triangle d f e
		   f _ e
			 \ |
			   d

		 c == d
		 b == f
		 Duplicate vertices, you should add indexing

		a = (x,y,z)
		b = (x, y+1)
		c = (x+1,y)

		d = (x+1,y)
		e = (x, y+1)
		f = (x+1,y+1)

		 each vertex a, b,c, etc. will have 5 data:
		 x y z u v
		  */

	for (int x = -width/2; x < width/2 - 1; x++) {
		float  offSetX = x * stepSize;
		for (int z = -height/2; z < height/2 - 1; z++) {
			float offSetZ = z * stepSize;
			makeVertex(offSetX, offSetZ);  // a
			makeVertex(offSetX, offSetZ + stepSize);  // b
			makeVertex(offSetX + stepSize, offSetZ);   // c
			makeVertex(offSetX + stepSize, offSetZ);  //d
			makeVertex(offSetX, offSetZ + stepSize);  //e
			makeVertex(offSetX + stepSize, offSetZ + stepSize);  //f
		}
	}

}

void Terrain::makeVertex(int x, int z) {

	//x y z position
	m_vertices.push_back((float)x); //xPos
	m_vertices.push_back(0.0f); //yPos - always 0 for now. Going to calculate this on GPU - could change to calclaute it here.
	m_vertices.push_back((float)z); //zPos

   // add texture coords
	m_vertices.push_back((float)x / (width * stepSize));
	m_vertices.push_back((float)z / (height * stepSize));


}
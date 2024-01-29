#pragma once
#include <glm/glm.hpp>

struct BaseLight
{
	unsigned int type;
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
};

struct DirectionalLight : BaseLight
{
	unsigned int type = 1 << 0; // ...0001
	glm::vec3 direction;

	float ambient;
};

struct PointLight : BaseLight
{
	unsigned int type = 1 << 1; // ...0010 
	glm::vec3 position;
	glm::vec3 attenuation;

	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;
};

struct SpotLight : BaseLight
{
	unsigned int type = 1 << 2; // ...0100 
	glm::vec3 direction;
	glm::vec3 position;
	glm::vec3 attenuation;

	float cutOff;
	float outerCutOff;
};


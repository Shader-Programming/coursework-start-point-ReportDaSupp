#version 440 core

uniform vec3 cubeColor;
uniform bool eLightsVisible;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

void main()
{
	if (eLightsVisible)
		FragColor = vec4(normalize(cubeColor), 1.0);
	else
		FragColor = FragColor;

	BrightColor = vec4(normalize(cubeColor), 1.0);
}
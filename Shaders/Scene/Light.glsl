#region Vertex

#version 440 core

layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}

#region Fragment

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

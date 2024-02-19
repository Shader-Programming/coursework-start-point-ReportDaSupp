#region Vertex

#version 430 core

layout(location = 0) in vec3 Vertex;

uniform mat4 CameraMatrix;

out vec3 Position;

void main()
{
    Position = Vertex.xyz;
    gl_Position = CameraMatrix * vec4(Vertex.xyz, 1.0);
}

#region Fragment

#version 430 core

out vec4 FragColor;

in vec3 Position;

uniform samplerCube SkyTexture;

void main()
{
    FragColor = vec4(textureCube(SkyTexture, Position).rgb,1.0);
}
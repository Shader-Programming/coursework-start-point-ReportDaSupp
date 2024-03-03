#region Vertex

#version 440 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;
out vec4 pos;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 viewPos;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos + viewPos, 1.0);
    gl_Position = pos.xyww;
}

#region Fragment

#version 440 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormalMap;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gNormal;
layout (location = 4) out vec3 gTBN1;
layout (location = 5) out vec3 gTBN2;
layout (location = 6) out vec3 gTBN3;

in vec3 TexCoords;
in vec4 pos;

uniform samplerCube skybox;

void main()
{    
    gPosition = pos;
    gAlbedoSpec = vec4(texture(skybox, TexCoords).rgb, 0.0f);
}
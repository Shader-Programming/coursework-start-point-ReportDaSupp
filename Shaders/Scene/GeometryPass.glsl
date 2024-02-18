#region Vertex

#version 440 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

out DATA {
    vec2 TexCoords;
    mat4 Model;
    mat4 Projection;
    vec3 Normal;
} data_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    data_out.TexCoords = aTexCoords;
    data_out.Model = model;
    data_out.Projection = projection * view;
    gl_Position = model * vec4(aPos, 1.0);
    data_out.Normal = aNormal;
}

#region Geometry

#version 440 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in DATA {
    vec2 TexCoords;
    mat4 Model;
    mat4 Projection;
    vec3 Normal;
} data_in[];

out vec3 FragPos;
out vec2 TexCoords;
out mat3 TBN;
out vec3 Normal;

void main()
{
    vec3 edge0 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 edge1 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;

    vec2 deltaUV0 = data_in[1].TexCoords - data_in[0].TexCoords;
    vec2 deltaUV1 = data_in[2].TexCoords - data_in[0].TexCoords;

    float invDet = 1.0f / (deltaUV0.x * deltaUV1.y - deltaUV1.x * deltaUV0.y);

    vec3 tangent = vec3(invDet * (deltaUV1.y * edge0 - deltaUV0.y * edge1));
    vec3 bitangent = vec3(invDet * (-deltaUV1.x * edge0 + deltaUV0.x * edge1));

    vec3 T = normalize(vec3(data_in[0].Model * vec4(tangent, 0.0f)));
    vec3 B = normalize(vec3(data_in[0].Model * vec4(bitangent, 0.0f)));
    vec3 N = normalize(vec3(data_in[0].Model * vec4(cross(edge1, edge0), 0.0f)));
    TBN = mat3(T, B, N);

    for(int i = 0; i < 3; i++)
    {
        FragPos = vec3(data_in[i].Projection * gl_in[i].gl_Position);
        gl_Position = data_in[i].Projection * gl_in[i].gl_Position;
        TexCoords = data_in[i].TexCoords;
        Normal = data_in[i].Normal;
        EmitVertex();
    }
    
    EndPrimitive();
}

#region Fragment

#version 440 core

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;
in vec3 Normal;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormalMap;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gNormal;
layout (location = 4) out vec3 gTBN1;
layout (location = 5) out vec3 gTBN2;
layout (location = 6) out vec3 gTBN3;


uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;

void main()
{   
    // G-buffer outputs
    gPosition = vec4(FragPos, 1.0);

    // Applying normal mapping
    gNormalMap = vec4(texture(normalTexture, TexCoords).rgb, 0.0f);
    gNormal = Normal;

    // Albedo and specular from textures
    vec3 albedo = texture(diffuseTexture, TexCoords).rgb;
    float specular = texture(specularTexture, TexCoords).r;
    gAlbedoSpec = vec4(albedo, specular);

    gTBN1 = vec3(TBN[0]);
    gTBN2 = vec3(TBN[1]);
    gTBN3 = vec3(TBN[2]);
}
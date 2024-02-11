#region Vertex

#version 440 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Tangent;
    vec3 NormalMat;
} vs_out;

out vec2 UV;
out vec3 normal;
out vec3 posInWS;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{   
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));

    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    vs_out.Tangent = T;
    vs_out.NormalMat = N;

    UV = aTexCoords;
    normal = N;
    posInWS = aPos;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

#region Geometry

#version 440 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Tangent;
    vec3 NormalMat;
} gs_in[];

out GS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Tangent;
    vec3 NormalMat;
} gs_out;

in vec2 UV[];
in vec3 normal[];
in vec3 posInWS[];

out vec2 gUV;
out vec3 gNormal;
out vec3 gPosInWS;

void main() {

    for (int i = 0; i < 3; i++)
    {
        gs_out.FragPos = gs_in[i].FragPos;
        gs_out.TexCoords = gs_in[i].TexCoords;
        gs_out.Tangent = gs_in[i].Tangent;
        gs_out.NormalMat = gs_in[i].NormalMat;

        gl_Position = gl_in[i].gl_Position;
        gUV = UV[i];
        gNormal = normal[i];
        gPosInWS = posInWS[i];
        EmitVertex();
    }

    EndPrimitive();

}

#region Fragment

#version 440 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gTangent;
layout (location = 4) out vec3 gNormalMat;

in GS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Tangent;
    vec3 NormalMat;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;

struct Material {
    vec3 diffuse;
    float specular;
    vec3 normal;
} material;

void main()
{
    // Normal Texture
    material.normal = texture(normalTexture, fs_in.TexCoords).rgb;
    material.normal = normalize(material.normal * 2.0 - 1.0);

    // Diffuse Texture
    material.diffuse = texture(diffuseTexture, fs_in.TexCoords).rgb;

    // Specular Texture
    material.specular = texture(specularTexture, fs_in.TexCoords).r;

    // Outputs
    gPosition = fs_in.FragPos;
    gNormal = material.normal;
    gAlbedoSpec.rgb = material.diffuse;
    gAlbedoSpec.a = material.specular;
    gTangent = fs_in.Tangent;
    gNormalMat = fs_in.NormalMat;
}
#version 440 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gTangent;
layout (location = 4) out vec3 gNormalMat;

in VS_OUT {
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
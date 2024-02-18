#region Vertex

#version 440 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;

uniform sampler2D gTBN1, gTBN2, gTBN3;

out vec2 TexCoords;
out mat3 TBN;

void main()
{
    // Construct the TBN matrix
    TBN = transpose(mat3(texture(gTBN1, aUV).rgb, texture(gTBN2, aUV).rgb, texture(gTBN3, aUV).rgb));
	TexCoords = aUV;
	gl_Position = vec4(aPos, 1.0);
}

#region Fragment

#version 440 core

in vec2 TexCoords;
in mat3 TBN;

uniform sampler2D gPosition, gNormalMap, gAlbedoSpec, gNormal;
uniform sampler2D ShadowMap;
uniform mat4 lightSpaceMatrix;
uniform vec3 viewPos;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct DirectionalLight {
    vec3 color;
    vec3 direction;
    float ambient;
};

struct PointLight {
    vec3 color;
    vec3 position;
    vec3 attenuation;
};

struct SpotLight {
    vec3 color;
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    vec3 attenuation;
};

uniform DirectionalLight dLight;
uniform PointLight pLights[32];
uniform SpotLight sLights[32];
uniform int numPointLights;
uniform int numSpotLights;

uniform bool eDirectional;
uniform bool ePointLight;
uniform bool eSpotLight;
uniform bool eRimming;

void main()
{
    // Sample G-buffer textures to retrieve fragment position, normal, and albedo/specular
    vec3 albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float spec = texture(gAlbedoSpec, TexCoords).a * texture(gNormalMap, TexCoords).r;
    FragColor = vec4(albedo * vec3(spec), 1.0f);
    BrightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

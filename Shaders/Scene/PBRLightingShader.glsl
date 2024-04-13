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

uniform sampler2D gPosition, gNormalMap, gAlbedoSpec, gAO;
uniform samplerCube skybox;
uniform vec3 viewPos;

layout (location = 0) out vec4 FragColor;

// Definitions for PBR lighting models and constants here
const float PI = 3.14159265359;

// Utility functions for PBR
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// PBR lighting calculation function here
vec3 calcPBRDirectLight(vec3 N, vec3 V, vec3 L, vec3 lightColor, float roughness, float metallic, vec3 albedo) {
    // Calculate Fresnel (Schlick approximation)
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 H = normalize(V + L);
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 nominator    = D * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; 
    vec3 specular = nominator / denominator;

    // Cook-Torrance BRDF
    return (specular + albedo * (1.0 - F) * (1.0 / PI)) * lightColor; 
}

vec3 calcDiffuseIBL(vec3 N, vec3 albedo) {
    vec3 irradiance = texture(skybox, N).rgb;
    return irradiance * albedo;
}

vec3 calcSpecularIBL(vec3 N, vec3 V, float roughness, vec3 F0) {
    vec3 R = reflect(-V, N);
    vec3 prefilteredColor = vec3(textureLod(skybox, R, roughness * log2(512.0)).rgb);
    vec2 brdfInput = vec2(max(dot(N, V), 0.0), roughness);
    vec2 brdf = vec2(texture(gAlbedoSpec, brdfInput).xy);
    return prefilteredColor * (F0 * brdf.x + brdf.y);
}

void main() {
    // Retrieve data from G-buffer
    vec3 fragPos = texture(gPosition, TexCoords).rgb;
    vec3 N = normalize(TBN * texture(gNormalMap, TexCoords).rgb); // Transform normal to world space using TBN
    vec3 albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float metallic = 0.1;
    float roughness = texture(gAlbedoSpec, TexCoords).a;
    float ao = 0.1;

    vec3 V = normalize(viewPos - fragPos); // View direction

    // Assume a single light source for this example
    vec3 lightPos = vec3(10.0, 10.0, 10.0); // Example light position
    vec3 L = normalize(lightPos - fragPos); // Light direction
    vec3 lightColor = vec3(1.0); // White light

    // Calculate direct lighting
    vec3 directLight = calcPBRDirectLight(N, V, L, lightColor, roughness, metallic, albedo);
    
    // Output color
    vec3 color = directLight + ao;
    
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 ambientDiffuse = calcDiffuseIBL(N, albedo);
    vec3 ambientSpecular = vec3(0.5);// = calcSpecularIBL(N, V, roughness, F0);

    vec3 ambient = ambientDiffuse + ambientSpecular;

    // Combine direct lighting with IBL
    vec3 finalColor = directLight * ambient;

    FragColor = vec4(finalColor, 1.0);

    
}

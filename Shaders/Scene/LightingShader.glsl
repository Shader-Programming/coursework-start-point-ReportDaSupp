#region Vertex

#version 440 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;

out vec2 TexCoords;

void main()
{
	TexCoords = aUV;
	gl_Position = vec4(aPos, 1.0);
}

#region Fragment

#version 440 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

struct DirectionalLight {
    vec3 color;
    vec3 direction;
};

uniform DirectionalLight dLight;

struct PointLight {
    vec3 color[32];
    vec3 position[32];
    vec3 attenuation[32];
    float numLights;
};

uniform PointLight pLight;

struct SpotLight {
    vec3 color[32];
    vec3 position[32];
    vec3 direction[32];
    vec3 attenuation[32];
    float cutOff[32];
    float outerCutOff[32];
    float numLights;
    bool enabled;
};

uniform SpotLight sLight;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gTangent;
uniform sampler2D gNormalMat;
uniform sampler2D ShadowMap;

uniform bool eDirectional;
uniform bool ePointLight;
uniform bool eSpotLight;
uniform bool eRimming;
uniform bool eDirectionalSM;

uniform mat4 lightSpaceMatrix;

uniform vec3 viewPos;

struct Material {
    vec3 diffuse;
    float specular;
    vec3 normal;
    float shininess;
} material;

vec4 FragPosLightSpace;
vec3 FragPos;
vec3 ViewPos;
mat3 TBN;

float ShadowCalculation()
{
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float bias = max(0.05 * (1.0 - dot(material.normal, normalize(-(TBN * dLight.direction)))), 0.005);

    float closestDepth = texture(ShadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z - bias;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(ShadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    return shadow;
}

vec3 calcDirectionalLighting()
{   
    vec3 direction = TBN * dLight.direction;

    // ambient
    vec3 ambient = 0.1 * material.diffuse;

    // diffuse
    vec3 lightDir = normalize(-direction);
    float diff = max(dot(lightDir, material.normal), 0.0);
    vec3 diffuse = dLight.color * diff * material.diffuse; 

    // specular
    vec3 viewDir = normalize(ViewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + material.normal);
    float spec = pow(max(dot(material.normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = dLight.color * spec * material.specular;
    if (eDirectionalSM)
        return vec3(ambient + (1-ShadowCalculation()) * (diffuse + specular));
    else
        return vec3(ambient + diffuse + specular);
}

vec3 calcPointLighting()
{   
    vec3 diffuse = vec3(0.0f, 0.0f, 0.0f);
    vec3 specular = vec3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < pLight.numLights; i++)
    {
        vec3 position = TBN * pLight.position[i];

        // diffuse 
        vec3 lightDir = normalize(position - FragPos);
        float diff = max(dot(material.normal, lightDir), 0.0);
        vec3 tempDiffuse = pLight.color[i] * diff * material.diffuse;
    
        // specular
        vec3 viewDir = normalize(ViewPos - FragPos);
        vec3 halfwayDir = normalize(lightDir + material.normal); 
        float spec = pow(max(dot(material.normal, halfwayDir), 0.0), material.shininess);
        vec3 tempSpecular = pLight.color[i] * spec * material.specular;  
    
        // attenuation
        float distance = length(position - FragPos);
        float attenuation = 1.0 / (pLight.attenuation[i].x + pLight.attenuation[i].y * distance + pLight.attenuation[i].z * (distance * distance)); 

        tempDiffuse *= attenuation;
        tempSpecular *= attenuation; 

        diffuse += (tempDiffuse);
        specular += (tempSpecular);
    }

    return vec3((diffuse + specular) / pLight.numLights);
}

vec3 calcSpotLighting()
{   
    vec3 diffuse = vec3(0.0f, 0.0f, 0.0f);
    vec3 specular = vec3(0.0f, 0.0f, 0.0f);
    

    for (int i = 0; i < sLight.numLights; i++)
    {
        vec3 position = TBN * sLight.position[i];
        vec3 direction = TBN * sLight.direction[i];

        vec3 lightDir = normalize(position - FragPos);
        float diff = max(dot(material.normal, lightDir), 0.0);
        vec3 tempDiffuse = sLight.color[i] * diff * material.diffuse;
    
        // specular
        vec3 viewDir = normalize(ViewPos - FragPos);
        vec3 halfwayDir = normalize(lightDir + material.normal);
        float spec = pow(max(dot(material.normal, halfwayDir), 0.0), material.shininess);
        vec3 tempSpecular = vec3(spec) * material.specular; 
    
        // spotlight (soft edges)
        float theta = dot(lightDir, normalize(-direction)); 
        float epsilon = (sLight.cutOff[i] - sLight.outerCutOff[i]);
        float intensity = clamp((theta - sLight.outerCutOff[i]) / epsilon, 0.0, 1.0);
        tempDiffuse *= intensity;
        tempSpecular *= intensity;
    
        // attenuation
        float distance = length(position - FragPos);
        float attenuation = 1.0 / (sLight.attenuation[i].x + sLight.attenuation[i].y * distance + sLight.attenuation[i].z * (distance * distance));    
        tempDiffuse *= attenuation;
        tempSpecular *= attenuation;

        diffuse += (tempDiffuse);
        specular += (tempSpecular);
    }

    return vec3((diffuse + specular) / sLight.numLights);
}

vec3 calcRimLighting()
{   
    vec3 n = material.normal;
    vec3 v = abs(normalize(ViewPos));     
    float vdn = 1.0 - max(dot(v, n), 0.0);
    vdn = smoothstep(0.95, 1.0, vdn);
    vdn = min(vdn, 0.2);
    return vec3(vdn);
}

void main()
{
    // Frag Position
    FragPos = texture(gPosition, TexCoords).rgb;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    // Normal Texture
    material.normal = texture(gNormal, TexCoords).rgb;

    // Diffuse Texture
    material.diffuse = texture(gAlbedoSpec, TexCoords).rgb;

    // Specular Texture
    material.specular = texture(gAlbedoSpec, TexCoords).a;
    material.shininess = 32.0;

    vec3 T = texture(gTangent, TexCoords).rgb;
    vec3 N = texture(gNormalMat, TexCoords).rgb;
    vec3 Tb = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, Tb);

    TBN = transpose(mat3(T, B, N));

    ViewPos = TBN * viewPos;
    FragPos = TBN * FragPos;

    // Lighting
    vec3 result = vec3(0);
    if (eDirectional)
        result += calcDirectionalLighting();
    if (ePointLight)
        result += calcPointLighting();
    if (eSpotLight)
        result += calcSpotLighting();
    if (eRimming)
        BrightColor += vec4(calcRimLighting(),1.0);

    FragColor = vec4(result, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    
    if(brightness > 1.0)
        BrightColor += vec4(FragColor.rgb, 1.0);
    else
        BrightColor += vec4(0.0, 0.0, 0.0, 1.0);

    
}

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
uniform bool eNormals;

vec3 calcDirectionalLighting(vec3 normalMap, vec3 albedo, float specularStrength, vec3 viewDir, vec3 fragPos)
{
    // ambient lighting
	float ambient = dLight.ambient;

	// diffuse lighting
	vec3 lightDirection = normalize(-dLight.direction);
	float diffuse = max(dot(normalize(normalMap), lightDirection), 0.0f);

	// specular lighting
	float specular = 0.0f;
	if (diffuse != 0.0f)
	{
		float specularLight = 0.50f;
		vec3 halfwayVec = normalize(viewDir + dLight.direction);
		float specAmount = pow(max(dot(normalMap, halfwayVec), 0.0f), 16);
		specular = specAmount * specularLight;
	};

	return (albedo * (diffuse + ambient) + specularStrength * specular) * dLight.color;
}

vec3 calcPointLighting(vec3 normalMap, vec3 albedo, float specularStrength, vec3 viewDir, vec3 fragPos)
{
    vec3 result = vec3(0.0);
    for (int i = 0; i < numPointLights; ++i)
    {
	    vec3 lightVec = (pLights[i].position - fragPos);

	    // intensity of light with respect to distance
	    float dist = length(lightVec);
	    float c = 0.032f;
	    float b = 0.09f;
	    float inten = 1.0f / (c * dist * dist + b * dist + 1.0f);

	    // diffuse lighting
	    vec3 lightDirection = normalize(lightVec);
	    float diffuse = max(dot(normalMap, lightDirection), 0.0f);

	    // specular lighting
	    float specular = 0.0f;
	    if (diffuse != 0.0f)
	    {
		    float specularLight = 0.50f;
		    vec3 halfwayVec = normalize(viewDir + lightDirection);
		    float specAmount = pow(max(dot(normalMap, halfwayVec), 0.0f), 16);
		    specular = specAmount * specularLight;
	    };

	    result += (albedo * (diffuse * inten) + specularStrength * specular * inten) * pLights[i].color;
    }
    return result;
}

void main()
{
    // Sample G-buffer textures to retrieve fragment position, normal, and albedo/specular
    vec3 fragPos = texture(gPosition, TexCoords).rgb;
    vec3 normalMap;
    if (eNormals)
        normalMap = normalize(texture(gNormal, TexCoords).rgb * 0.7 + texture(gNormalMap, TexCoords).rgb * 0.4);
    else
        normalMap = texture(gNormal, TexCoords).rgb;
    vec3 albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float specularStrength = texture(gAlbedoSpec, TexCoords).a;
    
    if (albedo != vec3(0.0))
    {

        // Define view direction
        vec3 viewDir = normalize(viewPos - fragPos);


        // Proceed with lighting calculations...
        vec3 result = vec3(0.0);

        // Directional lighting
        if (eDirectional)
        {
            result += calcDirectionalLighting(normalMap, albedo, specularStrength, viewDir, fragPos);
        }

        // Point lighting
        if (ePointLight)
        {
            result += calcPointLighting(normalMap, albedo, specularStrength, viewDir, fragPos);
        }

        // Spot lighting
        if (eSpotLight)
        {
            //result += calcSpotLighting(normalMap, albedo, specularStrength, viewDir, fragPos);
        }

        // Rim lighting
        if (eRimming)
        {
            //vec3 rimColor = calcRimLighting(viewDir, normalMap);
            //result += rimColor;
        }

        FragColor = vec4(result, 1.0);

        BrightColor = vec4(0.0); // Initialize to zero
        float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
        if (brightness > 1.0)
        {
            BrightColor = vec4(result, 1.0);
        }

    }
    else
    {
        FragColor = vec4(0.0);
        BrightColor = vec4(0.0);
    }
}

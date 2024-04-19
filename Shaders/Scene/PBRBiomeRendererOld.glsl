#region Vertex
#version 450 core
layout(location = 0) in vec3 aPos;

out VS_OUT {
    vec3 position;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    vs_out.position = vec3(vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

#region TesselationControl
#version 450 core
layout(vertices = 3) out;

in VS_OUT {
    vec3 position;
} tcs_in[];

out TCS_OUT {
    vec3 position;
} tcs_out[];

uniform vec3 cameraPos;
uniform mat4 model;
uniform samplerCube heightMap;

void main() {
    tcs_out[gl_InvocationID].position = tcs_in[gl_InvocationID].position;

    float distance = length(cameraPos - vec3(model * vec4(tcs_in[gl_InvocationID].position, 1.0)));

    float minTess = 4.0;
    float maxTess = 32.0;
    float minDistance = 1.0;
    float maxDistance = 300.0;

    float normalizedDistance = clamp((distance - minDistance) / (maxDistance - minDistance), 0.0, 1.0);

    float tessLevel = maxTess * exp(-2.0 * normalizedDistance);
    tessLevel = clamp(tessLevel, minTess, maxTess);

    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = tessLevel;
        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
    }
   
}

#region TesselationEvaluation
#version 450 core
layout(triangles, equal_spacing, ccw) in;

in TCS_OUT {
    vec3 position;
} tes_in[];

out vec3 tes_position;

uniform mat4 model;

void main() {
    vec3 p0 = gl_TessCoord.x * tes_in[0].position;
    vec3 p1 = gl_TessCoord.y * tes_in[1].position;
    vec3 p2 = gl_TessCoord.z * tes_in[2].position;
    vec3 pos = p0 + p1 + p2;
    
    tes_position = (vec4(pos, 1.0)).xyz; // Apply model matrix here
    gl_Position = vec4(pos, 1.0); // Apply transformation
}


#region Geometry
#version 450 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 tes_position[];
out vec3 FragPos;
out vec3 Normal;
out vec3 UV3;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform bool water;
uniform float elapsedTime;

uniform samplerCube heightMap;

float radius = 1;

vec3 calculateNormal(vec3 pos, vec3 normPosition) {
    float eps = 0.01;
    float heightCenter = texture(heightMap, normPosition).r;
    float heightX = texture(heightMap, normalize(normPosition + vec3(eps, 0, 0))).r;
    float heightY = texture(heightMap, normalize(normPosition + vec3(0, eps, 0))).r;
    float heightZ = texture(heightMap, normalize(normPosition + vec3(0, 0, eps))).r;

    vec3 gradient = vec3(heightX - heightCenter, heightY - heightCenter, heightZ - heightCenter) / eps;
    vec3 perturbedNormal = normalize(normPosition - gradient);

    return perturbedNormal;
}

void main() {
    vec3 normals[3];
    
    for (int i = 0; i < 3; i++) {
        vec3 normPosition = normalize(tes_position[i]);
        float displacement = texture(heightMap, normPosition).r;

        float effectiveRadius = 0.95 + displacement * 0.1;
        vec3 spherePos = normPosition * effectiveRadius;

        vec3 modelPosition = (vec4(spherePos, 1.0)).xyz;
        vec3 perturbedNormal = calculateNormal(modelPosition, normPosition);

        FragPos = modelPosition;
        Normal = perturbedNormal;
        UV3 = modelPosition;

        gl_Position = projection * view * model * vec4(modelPosition, 1.0);
        EmitVertex();
    }
    EndPrimitive();
}


#region Fragment
#version 450 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 UV3;
in vec3 biomeType;

uniform vec3 cameraPos;

uniform sampler2D beachAlbedo, beachAO, beachNormal, beachRoughness;
uniform sampler2D forestAlbedo, forestAO, forestNormal, forestRoughness;
uniform sampler2D grassAlbedo, grassAO, grassNormal, grassRoughness;
uniform sampler2D rockAlbedo, rockAO, rockNormal, rockRoughness;
uniform sampler2D sandAlbedo, sandAO, sandNormal, sandRoughness;
uniform sampler2D snowAlbedo, snowAO, snowNormal, snowRoughness;

uniform samplerCube heightMap;
uniform samplerCube tempMap;
uniform samplerCube precipMap;

vec2 UV2;


vec3 triplanarBlend(vec3 normal) {
    vec3 blend = abs(normal);
    blend /= (blend.x + blend.y + blend.z); // Normalize blend weights
    return blend;
}

vec2 selectTriplanarUV(vec3 worldPos, vec3 normal) {
    vec3 blendWeights = triplanarBlend(normal);
    float scale = 1.0; // Adjust scale to suit texture scaling needs

    // Calculate UV coordinates for each projection
    vec2 uvX = worldPos.yz * scale;
    vec2 uvY = worldPos.xz * scale;
    vec2 uvZ = worldPos.xy * scale;

    // Select UV coordinates based on the highest blend weight
    if (blendWeights.x > blendWeights.y && blendWeights.x > blendWeights.z) {
        return uvX;
    } else if (blendWeights.y > blendWeights.z) {
        return uvY;
    } else {
        return uvZ;
    }
}

vec3 getBiomeType(float height, float temperature, float precipitation) {
    // Environmental thresholds
    const float waterLevel = 0.45;
    const float mountainLevel = 0.7;
    const float snowTemperature = 0.2;
    const float forestPrecipitationThreshold = 0.3;
    const float desertTemperature = 0.6;
    const float desertPrecipitation = 0.2;

    // Biome encoding: 1-Sand, 2-Rock, 3-Snow, 4-Forest, 5-Grass, 6-Dirt
    int primaryBiome = 5; // Default to grass
    int secondaryBiome = 5; // Same as primary by default
    float blendFactor = 0.0;

    if (height < waterLevel) {
        // Underwater biome
        primaryBiome = 1; // Sand
        secondaryBiome = 6; // Dirt as secondary biome under deeper water
        blendFactor = smoothstep(0.40, 0.45, height); // Smooth transition at near water edges
    } else {
        // Above water biomes
        if (height > mountainLevel) {
            // High altitude biomes
            primaryBiome = 2; // Rock
            if (temperature < snowTemperature) {
                secondaryBiome = 3; // Snow in cold high altitudes
                blendFactor = smoothstep(0.1, 0.2, temperature); // Smooth transition into snow
            }
        } else {
            // Normal altitude biomes
            if (precipitation > forestPrecipitationThreshold) {
                primaryBiome = 4; // Forest
                blendFactor = smoothstep(0.25, 0.35, precipitation); // Gradual increase in forest density
            } else {
                if (temperature > desertTemperature && precipitation < desertPrecipitation) {
                    primaryBiome = 1; // Desert sand
                    blendFactor = smoothstep(0.15, 0.25, temperature); // More pronounced desert features
                }
            }
        }
    }

    // Calculate blend factor based on environmental context
    blendFactor = clamp(blendFactor, 0.0, 1.0); // Ensure blend factor remains within bounds
    return vec3(float(primaryBiome), float(secondaryBiome), blendFactor);
}

void getBiomeTextures(float biomeIndex, out vec3 albedo, out vec3 normal, out float roughness, out float ao) {
    switch (int(biomeIndex)) {
        case 1:
            albedo = texture(sandAlbedo, UV2).rgb;
            normal = texture(sandNormal, UV2.xy).rgb;
            roughness = texture(sandRoughness, UV2.xy).r;
            ao = texture(sandAO, UV2.xy).r; 
            break;
        case 2:
            albedo = texture(rockAlbedo, UV2.xy).rgb;
            normal = texture(rockNormal, UV2.xy).rgb;
            roughness = texture(rockRoughness, UV2.xy).r;
            ao = texture(rockAO, UV2.xy).r;
            break;
        case 3:
            albedo = texture(snowAlbedo, UV2.xy).rgb;
            normal = texture(snowNormal, UV2.xy).rgb;
            roughness = texture(snowRoughness, UV2.xy).r;
            ao = texture(snowAO, UV2.xy).r; 
            break;
        case 4:
            albedo = texture(forestAlbedo, UV2.xy).rgb;
            normal = texture(forestNormal, UV2.xy).rgb;
            roughness = texture(forestRoughness, UV2.xy).r;
            ao = texture(forestAO, UV2.xy).r; 
            break;
        case 5:
            albedo = texture(grassAlbedo, UV2.xy).rgb;
            normal = texture(grassNormal, UV2.xy).rgb;
            roughness = texture(grassRoughness, UV2.xy).r;
            ao = texture(grassAO, UV2.xy).r;
            break;
        case 6:
            albedo = texture(beachAlbedo, UV2.xy).rgb;
            normal = texture(beachNormal, UV2.xy).rgb;
            roughness = texture(beachRoughness, UV2.xy).r;
            ao = texture(beachAO, UV2.xy).r;
            break;
        default:
            albedo = texture(sandAlbedo, UV2.xy).rgb;
            normal = texture(sandNormal, UV2.xy).rgb;
            roughness = texture(sandRoughness, UV2.xy).r;
            ao = texture(sandAO, UV2.xy).r;
            break;
    }
}

void main() {
    float height = texture(heightMap, UV3).r;
    float temperature = texture(tempMap, UV3).r;
    float precipitation = texture(precipMap, UV3).r;

    UV2 = selectTriplanarUV(UV3, Normal);

    vec3 biomeType = getBiomeType(height, temperature, precipitation);

    vec3 albedo1, normal1;
    float roughness1, ao1;
    vec3 albedo2, normal2;
    float roughness2, ao2;

    getBiomeTextures(biomeType.x, albedo1, normal1, roughness1, ao1);
    getBiomeTextures(biomeType.y, albedo2, normal2, roughness2, ao2);

    vec3 albedoMix = mix(albedo1, albedo2, biomeType.z);
    vec3 normalMix = normalize(mix(normal1, normal2, biomeType.z) * 2.0 - 1.0);
    float roughnessMix = mix(roughness1, roughness2, biomeType.z);
    float aoMix = mix(ao1, ao2, biomeType.z);

    vec3 lightDir = normalize(vec3(0.5, 1.0, -0.5));
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = diff * albedoMix;
    vec3 ambient = 0.3 * albedoMix;//aoMix * albedoMix;

    FragColor = vec4(diffuse + ambient, 1.0);
}
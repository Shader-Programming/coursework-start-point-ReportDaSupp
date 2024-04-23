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
    
    tes_position = (vec4(pos, 1.0)).xyz;
    gl_Position = vec4(pos, 1.0);
}


#region Geometry
#version 450 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 tes_position[];
out vec3 FragPos;
out vec3 Normal;
out vec3 UV3;
out mat3 TBN;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform bool sphere;

uniform samplerCube heightMap;

vec3 calculateGradient(vec3 pos, float epsilon) {
    float heightCenter = texture(heightMap, pos).r;
    float heightXPlus = texture(heightMap, pos + vec3(epsilon, 0.0, 0.0)).r;
    float heightXMinus = texture(heightMap, pos - vec3(epsilon, 0.0, 0.0)).r;
    float heightYPlus = texture(heightMap, pos + vec3(0.0, epsilon, 0.0)).r;
    float heightYMinus = texture(heightMap, pos - vec3(0.0, epsilon, 0.0)).r;
    float heightZPlus = texture(heightMap, pos + vec3(0.0, 0.0, epsilon)).r;
    float heightZMinus = texture(heightMap, pos - vec3(0.0, 0.0, epsilon)).r;

    // Central difference method to approximate derivatives
    float dX = (heightXPlus - heightXMinus) / (2.0 * epsilon);
    float dY = (heightYPlus - heightYMinus) / (2.0 * epsilon);
    float dZ = (heightZPlus - heightZMinus) / (2.0 * epsilon);

    return normalize(vec3(dX, dY, dZ));
}

void main() {
    vec3 normals[3];
    vec3 tangents[3];
    vec3 bitangents[3];
    float eps = 0.1;

    for (int i = 0; i < 3; i++) {
        vec3 normPosition = normalize(tes_position[i]);
        float displacement = texture(heightMap, (vec4(normPosition, 1.0)).xyz).r;
        float effectiveRadius = 0.95 + displacement * 0.1;
        vec3 spherePos = normPosition * effectiveRadius;
        vec3 modelPosition = (vec4(spherePos, 1.0)).xyz;

        if (!sphere)
        {
            displacement = texture(heightMap, (vec4(tes_position[i], 1.0)).xyz).r;
            spherePos = tes_position[i] + displacement * 0.1;
            modelPosition = (vec4(spherePos, 1.0)).xyz;
        }

        vec3 gradient = calculateGradient(modelPosition, eps);
        vec3 perturbedNormal = normalize(modelPosition - gradient);
        vec3 tangent = normalize(vec3(-perturbedNormal.y, perturbedNormal.x, 0.0));
        vec3 bitangent = cross(perturbedNormal, tangent);

        normals[i] = perturbedNormal;
        tangents[i] = tangent;
        bitangents[i] = bitangent;

        FragPos = modelPosition;
        Normal = perturbedNormal;
        UV3 = spherePos;

        TBN = mat3(tangents[i], bitangents[i], normals[i]);
        gl_Position = projection * view * model * vec4(spherePos, 1.0);
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
in mat3 TBN;

uniform vec3 cameraPos;

uniform sampler2DArray AlbedoArray; 
uniform sampler2DArray AOArray; 
uniform sampler2DArray NormalArray;
uniform sampler2DArray RoughnessArray;

uniform samplerCube heightMap;
uniform samplerCube tempMap;
uniform samplerCube precipMap;

vec2 UV2;

struct BiomeWeights {
    float dirt;
    float forest;
    float grass;
    float rock;
    float sand;
    float snow;
};

struct TexData {
    vec3 Albedo;
    vec3 Normal;
    float Roughness;
    float AO;
};

vec2 selectTriplanarUV(vec3 worldPos, vec3 normal) {
    worldPos = normalize(worldPos);

    // Determine the maximum component of the normal vector
    vec3 absNormal = abs(normal);
    float maxComponent = max(max(absNormal.x, absNormal.y), absNormal.z);

    vec2 uv;

    if (maxComponent == absNormal.x) {
        uv = worldPos.yz;  // X is dominant
    } else if (maxComponent == absNormal.y) {
        uv = worldPos.xz;  // Y is dominant
    } else {
        uv = worldPos.xy;  // Z is dominant
    }

    // Scale and translate the UVs from [-1, 1] to [0, 1]
    uv = uv * 0.5 + 0.5;
    uv = uv * 16;

    return uv;
}


BiomeWeights calculateBiomeWeights(float height, float temperature, float precipitation) {
    BiomeWeights weights;

    const float baseWeight = 0.01;
    weights.sand = baseWeight;
    weights.dirt = baseWeight;
    weights.forest = baseWeight;
    weights.grass = baseWeight;
    weights.rock = baseWeight;
    weights.snow = baseWeight;

    // Environmental thresholds
    const float waterLevel = 0.48;
    const float snowTemperatureThreshold = 0.1;
    const float highAltitude = 0.75;
    const float desertTemperature = 0.75;
    const float desertPrecipitation = 0.1;

    // Transitions based on height
    if (height < waterLevel) {
        float depthFactor = smoothstep(0.4, 0.48, height);
        weights.sand = depthFactor;
        weights.dirt = 1.0 - depthFactor;
    } else {
        // Handling above water level
        if (height > highAltitude) {
            weights.rock = smoothstep(0.6, 0.75, height);
            if (temperature < snowTemperatureThreshold) {
                weights.snow = smoothstep(0.0, 0.1, temperature);
                weights.rock = 1.0 - weights.snow;
            }
        } else {
            // Grass and forest management based on precipitation
            if (precipitation > 0.15) {
                weights.forest = smoothstep(0.15, 0.25, precipitation);
                weights.grass = 1.0 - weights.forest;
            } else {
                weights.grass = 1.0;
            }

            // Handling desert conditions
            if (temperature > desertTemperature && precipitation < desertPrecipitation) {
                weights.sand = smoothstep(0.75, 0.85, temperature) * (1.0 - smoothstep(0.0, 0.1, precipitation));
                weights.grass *= (1.0 - weights.sand);
            }
        }
    }

    float totalWeight = weights.sand + weights.dirt + weights.forest + weights.grass + weights.rock + weights.snow;
    if (totalWeight > 0) {
        weights.sand /= totalWeight;
        weights.dirt /= totalWeight;
        weights.forest /= totalWeight;
        weights.grass /= totalWeight;
        weights.rock /= totalWeight;
        weights.snow /= totalWeight;
    }

    return weights;
}

TexData blendTextures(vec2 coord, BiomeWeights weights) {
    TexData texData;
    texData.Albedo += texture(AlbedoArray,   vec3(coord, 4)).rgb * weights.sand;
    texData.Albedo += texture(AlbedoArray,   vec3(coord, 3)).rgb * weights.rock;
    texData.Albedo += texture(AlbedoArray,   vec3(coord, 5)).rgb * weights.snow;
    texData.Albedo += texture(AlbedoArray,   vec3(coord, 1)).rgb * weights.forest;
    texData.Albedo += texture(AlbedoArray,   vec3(coord, 2)).rgb * weights.grass;
    texData.Albedo += texture(AlbedoArray,   vec3(coord, 0)).rgb * weights.dirt;

    texData.Normal += (texture(NormalArray, vec3(coord, 4)).rgb * 2 - 1) * weights.sand;
    texData.Normal += (texture(NormalArray, vec3(coord, 3)).rgb * 2 - 1) * weights.rock;
    texData.Normal += (texture(NormalArray, vec3(coord, 5)).rgb * 2 - 1) * weights.snow;
    texData.Normal += (texture(NormalArray, vec3(coord, 1)).rgb * 2 - 1) * weights.forest;
    texData.Normal += (texture(NormalArray, vec3(coord, 2)).rgb * 2 - 1) * weights.grass;
    texData.Normal += (texture(NormalArray, vec3(coord, 0)).rgb * 2 - 1) * weights.dirt;

    texData.Roughness += texture(RoughnessArray, vec3(coord, 4)).r * weights.sand;
    texData.Roughness += texture(RoughnessArray, vec3(coord, 3)).r * weights.rock;
    texData.Roughness += texture(RoughnessArray, vec3(coord, 5)).r * weights.snow;
    texData.Roughness += texture(RoughnessArray, vec3(coord, 1)).r * weights.forest;
    texData.Roughness += texture(RoughnessArray, vec3(coord, 2)).r * weights.grass;
    texData.Roughness += texture(RoughnessArray, vec3(coord, 0)).r * weights.dirt;

    texData.AO += texture(AOArray, vec3(coord, 4)).r * weights.sand;
    texData.AO += texture(AOArray, vec3(coord, 3)).r * weights.rock;
    texData.AO += texture(AOArray, vec3(coord, 5)).r * weights.snow;
    texData.AO += texture(AOArray, vec3(coord, 1)).r * weights.forest;
    texData.AO += texture(AOArray, vec3(coord, 2)).r * weights.grass;
    texData.AO += texture(AOArray, vec3(coord, 0)).r * weights.dirt;

    return texData;
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159 * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 calculatePBR(vec3 albedo, float metallic, vec3 normal, float roughness, vec3 lightPos, vec3 viewPos, float ao) {
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 N = normalize(TBN * normal);
    vec3 V = normalize(viewPos - FragPos);
    vec3 L = normalize(lightPos - FragPos);
    vec3 H = normalize(V + L);

    float NDF = DistributionGGX(N, H, roughness);  // Normal Distribution Function
    float G = GeometrySmith(N, V, L, roughness);  // Geometry function
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);  // Fresnel equation

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;  // Prevent division by zero
    vec3 specular = numerator / denominator;

    float NdotL = abs(max(dot(N, L), 0.0));
    vec3 diffuse = kD * albedo / 3.14159;

    return albedo * (diffuse + specular) + NdotL * ao;  // Apply ambient occlusion
}

void main() {
    float height = texture(heightMap, UV3).r;
    float temperature = texture(tempMap, UV3).r;
    float precipitation = texture(precipMap, UV3).r;

    UV2 = selectTriplanarUV(UV3, Normal);

    BiomeWeights biome = calculateBiomeWeights(height, temperature, precipitation);

    TexData ColorData;
    ColorData = blendTextures(UV2, biome);
    float metallic = (1 -ColorData.Roughness) + 0.25;
    vec3 lightPos = vec3(10, 200, 200);

    FragColor = vec4(calculatePBR(ColorData.Albedo, metallic, ColorData.Normal, ColorData.Roughness, lightPos, cameraPos, ColorData.AO), 1.0);
    FragColor = mix(FragColor, vec4(temperature, 0, 1-temperature, 1.0), 0.12);
    //FragColor = vec4(ColorData.Albedo, 1.0);
    //FragColor = vec4(ColorData.Normal, 1.0);
    //FragColor = vec4(normalize(TBN * ColorData.Normal), 1.0);
    //FragColor = vec4(TBN[0], 1.0);
    //FragColor = vec4(TBN[1], 1.0);
    //FragColor = vec4(TBN[2], 1.0);
    //FragColor = vec4(vec3(metallic), 1.0);
    //FragColor = vec4(vec3(ColorData.Roughness), 1.0);
    //FragColor = vec4(vec3(ColorData.AO), 1.0);
}
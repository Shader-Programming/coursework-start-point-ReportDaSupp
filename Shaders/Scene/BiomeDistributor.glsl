#region Compute
#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 6) in;

layout(binding = 0, r32f) uniform writeonly imageCube heightMap;
layout(binding = 1, r32f) uniform writeonly imageCube temperatureMap;
layout(binding = 2, r32f) uniform writeonly imageCube precipitationMap;

uniform float heightSeed = 0.1;

int hash(int x, int y, int z, float seed) {
    int n = x + y * 57 + z * 3623 + int(seed * 43758.5453);
    n = (n << 13) ^ n;
    return (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
}

vec3 gradients[16] = vec3[](
    vec3(1,1,0), vec3(-1,1,0), vec3(1,-1,0), vec3(-1,-1,0),
    vec3(1,0,1), vec3(-1,0,1), vec3(1,0,-1), vec3(-1,0,-1),
    vec3(0,1,1), vec3(0,-1,1), vec3(0,1,-1), vec3(0,-1,-1),
    vec3(1,1,0), vec3(-1,1,0), vec3(0,-1,1), vec3(0,-1,-1)
);

vec3 getGradient(int x, int y, int z, float seed) {
    int h = hash(x, y, z, seed);
    return gradients[h % 16];
}

float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);  // 6t^5 - 15t^4 + 10t^3
}

float lerp(float a, float b, float t) {
    return (1.0 - t) * a + t * b;
}

float perlinNoise(vec3 coord, float scale, float seed) {
    coord = coord * scale;  // Scale and seed the coordinates
    ivec3 P = ivec3(floor(coord));
    vec3 f = fract(coord);
    f = f * f * (3.0 - 2.0 * f);  // Smoothing curve

    float n000 = dot(getGradient(P.x, P.y, P.z, seed), f);
    float n001 = dot(getGradient(P.x, P.y, P.z+1, seed), f - vec3(0,0,1));
    float n010 = dot(getGradient(P.x, P.y+1, P.z, seed), f - vec3(0,1,0));
    float n011 = dot(getGradient(P.x, P.y+1, P.z+1, seed), f - vec3(0,1,1));
    float n100 = dot(getGradient(P.x+1, P.y, P.z, seed), f - vec3(1,0,0));
    float n101 = dot(getGradient(P.x+1, P.y, P.z+1, seed), f - vec3(1,0,1));
    float n110 = dot(getGradient(P.x+1, P.y+1, P.z, seed), f - vec3(1,1,0));
    float n111 = dot(getGradient(P.x+1, P.y+1, P.z+1, seed), f - vec3(1,1,1));

    float nx00 = lerp(n000, n100, f.x);
    float nx01 = lerp(n001, n101, f.x);
    float nx10 = lerp(n010, n110, f.x);
    float nx11 = lerp(n011, n111, f.x);
    float nxy0 = lerp(nx00, nx10, f.y);
    float nxy1 = lerp(nx01, nx11, f.y);
    return lerp(nxy0, nxy1, f.z);
}

float perlinNoiseLayered(vec3 coord, float scale, float seed)
{
    float baseNoise = perlinNoise(coord, (scale + seed) * 1.5, seed);
    float fineNoise = perlinNoise(coord, (scale + seed) * 8, seed);
    float finestNoise = perlinNoise(coord, (scale + seed) * 24, seed);

    baseNoise /= 1.75;
    baseNoise += fineNoise / 64;
    baseNoise += finestNoise / 256;
    return baseNoise;
}

vec3 getDirection(uint faceIndex, vec2 uv) {
    uv = uv * 2.0 - 1.0; // Transform uv to range [-1, 1]
    switch (faceIndex) {
        case 0: return vec3(1.0, -uv.y, -uv.x); // +X face
        case 1: return vec3(-1.0, -uv.y, uv.x); // -X face
        case 2: return vec3(uv.x, 1.0, uv.y); // +Y face
        case 3: return vec3(uv.x, -1.0, -uv.y); // -Y face
        case 4: return vec3(uv.x, -uv.y, 1.0); // +Z face
        case 5: return vec3(-uv.x, -uv.y, -1.0); // -Z face
    }
    return vec3(0);
}

float calculateTemperature(vec3 direction, float height) {
    float latitudeFactor = 1.0 - abs(direction.y); // Ranges from 0 at poles to 1 at equator
    float altitudeEffect = -0.005 * height; // Temperature decreases with altitude
    return latitudeFactor + altitudeEffect;
}

float calculatePrecipitation(float height, vec3 direction, float baseMoisture) {
    float moisture = baseMoisture;
    moisture += smoothstep(0.8, 0.3, height) * 0.3;
    return moisture;
}

void main() {
    uint faceIndex = gl_GlobalInvocationID.z;
    ivec3 xyz = ivec3(gl_GlobalInvocationID.xyz);
    vec2 uv = (vec2(xyz.xy) + 0.5) / vec2(imageSize(heightMap).xy);
    vec3 direction = getDirection(faceIndex, uv);

    float heightNoise = perlinNoiseLayered(direction, 1.0, heightSeed) * 5;

    float baseMoisture = perlinNoiseLayered(direction, 1.0, heightSeed) * 2;

    float temperatureNoise = calculateTemperature(direction, heightNoise);
    float precipitationNoise = calculatePrecipitation(heightNoise, direction, baseMoisture);

    imageStore(heightMap, xyz, vec4(abs(heightNoise), 0, 0, 1.0));
    imageStore(temperatureMap, xyz, vec4(abs(temperatureNoise), 0, 0, 1.0));
    imageStore(precipitationMap, xyz, vec4(precipitationNoise, 0, 0, 1.0));
}
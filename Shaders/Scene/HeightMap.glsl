#region Compute

#version 440 core

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 0, rgba32f) uniform image2D heightMap;
layout(binding = 1, r32f) uniform image2D DuDvMap;

uniform float ellapsedTime;
uniform vec3 viewPos;

const int perm[512] = int[512](
181,114,197,219,253,74,160,72,12,238,13,104,133,121,190,113,236,99,155,195,5,39,60,89,76,192,92,80,217,250,237,123,107,124,200,208,198,156,173,67,162,28,247,78,246,185,127,167,201,3,224,141,16,10,87,48,171,240,15,184,122,166,9,6,202,115,209,43,125,223,235,32,112,234,47,189,165,153,218,40,116,186,119,7,55,69,128,25,214,207,100,137,35,105,163,169,118,14,110,206,90,85,53,245,93,205,36,98,83,41,179,229,249,52,180,132,149,241,251,194,174,145,96,130,143,242,54,172,146,22,34,199,38,30,51,220,176,152,106,50,46,68,71,120,95,79,8,221,140,244,11,94,19,239,158,23,135,212,101,0,213,29,117,1,26,138,203,108,252,255,129,157,24,59,228,61,144,168,31,150,193,17,111,187,64,27,66,65,226,131,191,91,77,254,182,175,204,243,86,4,37,20,42,49,142,103,233,82,44,21,139,230,183,215,102,45,222,211,196,177,126,227,232,248,231,136,151,63,225,84,154,58,33,188,147,210,75,178,161,134,62,216,70,97,170,159,56,2,57,148,109,81,73,18,88,164,
181,114,197,219,253,74,160,72,12,238,13,104,133,121,190,113,236,99,155,195,5,39,60,89,76,192,92,80,217,250,237,123,107,124,200,208,198,156,173,67,162,28,247,78,246,185,127,167,201,3,224,141,16,10,87,48,171,240,15,184,122,166,9,6,202,115,209,43,125,223,235,32,112,234,47,189,165,153,218,40,116,186,119,7,55,69,128,25,214,207,100,137,35,105,163,169,118,14,110,206,90,85,53,245,93,205,36,98,83,41,179,229,249,52,180,132,149,241,251,194,174,145,96,130,143,242,54,172,146,22,34,199,38,30,51,220,176,152,106,50,46,68,71,120,95,79,8,221,140,244,11,94,19,239,158,23,135,212,101,0,213,29,117,1,26,138,203,108,252,255,129,157,24,59,228,61,144,168,31,150,193,17,111,187,64,27,66,65,226,131,191,91,77,254,182,175,204,243,86,4,37,20,42,49,142,103,233,82,44,21,139,230,183,215,102,45,222,211,196,177,126,227,232,248,231,136,151,63,225,84,154,58,33,188,147,210,75,178,161,134,62,216,70,97,170,159,56,2,57,148,109,81,73,18,88,164
);

// Gradient directions for 2D. These could be randomized or even static directions.
vec2 grad2[8] = vec2[](vec2(1,0), vec2(-1,0), vec2(0,1), vec2(0,-1),
                       vec2(1,1), vec2(-1,1), vec2(1,-1), vec2(-1,-1));

// Hash function
int hash(int x, int y) {
    return perm[(perm[x & 255] + y) & 255];
}

// Gradient function
float grad(int hash, float x, float y) {
    vec2 g = grad2[hash & 7];
    return x*g.x + y*g.y;
}

// Fade function as defined by Ken Perlin
float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// Linear interpolation
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Perlin noise function for 2D
float perlin(float x, float y, float et) {
    if (et > 0)
    {
        x += et * 0.15;
        y += et * 0.15;
    }
    int xi = int(floor(x)) & 255;
    int yi = int(floor(y)) & 255;
    float xf = x - floor(x);
    float yf = y - floor(y);
    float u = fade(xf);
    float v = fade(yf);
    int aa, ab, ba, bb;
    aa = hash(xi, yi);
    ab = hash(xi, yi + 1);
    ba = hash(xi + 1, yi);
    bb = hash(xi + 1, yi + 1);

    float x1, x2, y1;
    x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1, yf), u);
    x2 = lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u);
    y1 = lerp(x1, x2, v);
    return (y1 + 1.0) / 2.0; // Normalize to [0, 1]
}

vec3 calculateNormal(ivec2 coords) {
    float hC = perlin(float(coords.x), float(coords.y), 0);
    float hL = perlin(float(coords.x - 1), float(coords.y), 0);
    float hR = perlin(float(coords.x + 1), float(coords.y), 0);
    float hD = perlin(float(coords.x), float(coords.y - 1), 0);
    float hU = perlin(float(coords.x), float(coords.y + 1), 0);

    // Calculate gradient vectors
    vec3 dx = vec3(1.0, 0.0, hR - hL);
    vec3 dy = vec3(0.0, 1.0, hU - hD);

    // Cross product of gradients gives normal
    vec3 normal = normalize(cross(dx, dy));
    return normal;
}

void main() {
    ivec2 texelCoords = ivec2(gl_GlobalInvocationID.xy);
    float n = perlin(float(texelCoords.x) / 100.0, float(texelCoords.y) / 100.0, 0);

    vec3 normal = calculateNormal(texelCoords / 100);

    vec3 smoothNormal = vec3(0.0);
    int count = 0;
    for(int dx = -1; dx <= 1; ++dx) {
        for(int dy = -1; dy <= 1; ++dy) {
            if(dx == 0 && dy == 0) continue; // Skip the center pixel
            vec3 adjNormal = calculateNormal((texelCoords + ivec2(dx, dy))/100);
            smoothNormal += adjNormal;
            ++count;
        }
    }
    smoothNormal = normalize(smoothNormal / float(count));
    float d = perlin(float(texelCoords.x)/100, float(texelCoords.y) / 100.0, ellapsedTime);
    vec4 heightNormal;
    heightNormal = vec4(smoothNormal * 0.5 + 0.5, n);
    imageStore(heightMap, texelCoords, heightNormal);
    imageStore(DuDvMap, texelCoords, vec4(d, 0, 0, 0));
}
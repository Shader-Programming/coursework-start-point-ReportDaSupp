#region Compute

#version 440 core

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

struct Vertex {
    vec4 position;
    vec4 texCoords;
    vec4 normal;
};

layout(std430, binding = 0) buffer Vertices {
    Vertex vertices[];
};

uniform int gridWidth;
uniform int gridHeight;

void main() {
    uint idxX = gl_GlobalInvocationID.x;
    uint idxZ = gl_GlobalInvocationID.y;

    if (idxX >= gridWidth || idxZ >= gridHeight) return;

    uint index = (idxZ * gridWidth + idxX) * 6;

    float x = float(idxX);
    float z = float(idxZ);
    float nextX = x + 1.0;
    float nextZ = z + 1.0;

    // Calculate texture coordinates
    vec2 texCoords = vec2(x / float(gridWidth), z / float(gridHeight));
    vec2 texCoordsNextX = vec2(nextX / float(gridWidth), z / float(gridHeight));
    vec2 texCoordsNextZ = vec2(x / float(gridWidth), nextZ / float(gridHeight));
    vec2 texCoordsNext = vec2(nextX / float(gridWidth), nextZ / float(gridHeight));

    vec4 normal = vec4(0, 1, 0, 0);

    // Triangle 1
    vertices[index + 0].position = vec4(x, 0.0, z, 1.0)* 20;
    vertices[index + 0].texCoords.xy = texCoords;
    vertices[index + 0].normal = normal;
    
    vertices[index + 1].position = vec4(nextX, 0.0, z, 1.0)* 20;
    vertices[index + 1].texCoords.xy = texCoordsNextX;
    vertices[index + 0].normal = normal;
    
    vertices[index + 2].position = vec4(x, 0.0, nextZ, 1.0)* 20;
    vertices[index + 2].texCoords.xy = texCoordsNextZ;
    vertices[index + 0].normal = normal;

    // Triangle 2
    vertices[index + 3].position = vec4(nextX, 0.0, z, 1.0)* 20;
    vertices[index + 3].texCoords.xy = texCoordsNextX;
    vertices[index + 0].normal = normal;
    
    vertices[index + 4].position = vec4(nextX, 0.0, nextZ, 1.0) * 20;
    vertices[index + 4].texCoords.xy = texCoordsNext;
    vertices[index + 0].normal = normal;
    
    vertices[index + 5].position = vec4(x, 0.0, nextZ, 1.0)* 20;
    vertices[index + 5].texCoords.xy = texCoordsNextZ;
    vertices[index + 0].normal = normal;
}
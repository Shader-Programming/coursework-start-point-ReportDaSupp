#region Compute
#version 450 core
layout(local_size_x = 8, local_size_y = 8) in;

struct Vertex {
    vec4 position;
};

layout(std430, binding = 0) buffer VertexBuffer {
    Vertex vertices[];
};

uniform int subdivisions;

void main() {
    uint idx = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * 8;
    if (idx >= 6 * (subdivisions + 1) * (subdivisions + 1)) return;

    uint face = idx / ((subdivisions + 1) * (subdivisions + 1));
    uint faceIdx = idx % ((subdivisions + 1) * (subdivisions + 1));
    uint row = faceIdx / (subdivisions + 1);
    uint col = faceIdx % (subdivisions + 1);

    float offset = 1.0 / float(subdivisions);
    float x = -1.0 + col * offset * 2.0;
    float y = -1.0 + row * offset * 2.0;
    float z = 1.0;  // Always start with 1.0 which can be negated based on the face

    vec3 position;
    switch (face) {
        case 0: // Front face
            position = vec3(x, y, z);
            break;
        case 1: // Back face
            position = vec3(-x, y, -z);
            break;
        case 2: // Top face
            position = vec3(x, z, -y);
            break;
        case 3: // Bottom face
            position = vec3(x, -z, y);
            break;
        case 4: // Right face
            position = vec3(z, y, -x);
            break;
        case 5: // Left face
            position = vec3(-z, y, x);
            break;
    }

    vertices[idx].position = vec4(position, 1.0);
}
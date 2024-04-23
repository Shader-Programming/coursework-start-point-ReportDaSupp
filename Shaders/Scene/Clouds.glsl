#region Vertex
#version 450 core
layout(location = 0) in vec3 aPos;  // Position of the cloud center

out VS_OUT {
    vec3 worldPos;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 worldPosition = model * vec4(1, 1, 1, 1.0);
    vs_out.worldPos = worldPosition.xyz;
    gl_Position = projection * view * worldPosition;
}

#region Geometry
#version 450 core
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VS_OUT {
    vec3 worldPos;
} gs_in[];

out vec2 TexCoords;
out float texLayerIndex;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPosition;
uniform float size = 25.0;

vec3 planetCenter = vec3(0,0,0);

vec3 globalUp = vec3(0, 1, 0);

void main() {
    vec3 planetToCloudDir = normalize(gs_in[0].worldPos - planetCenter);
    vec3 viewDir = normalize(viewPosition - gs_in[0].worldPos);

    // Establish local coordinate system
    vec3 right = normalize(cross(planetToCloudDir, globalUp));
    vec3 forward = normalize(cross(right, planetToCloudDir));
    vec3 up = planetToCloudDir;  // Local 'up' is the 'planetToCloudDir'

    // Compute viewing angles
    float elevation = degrees(asin(dot(viewDir, up)));
    float azimuth = degrees(atan(dot(viewDir, forward), dot(viewDir, right))) + 180.0;

    // Map elevation and azimuth to texture indices
    float texLayerIndexY = round((elevation + 90.0) / 30.0);
    float texLayerIndexXZ = round(azimuth / 30.0);

    texLayerIndex = 84 - ((texLayerIndexY * 12) + texLayerIndexXZ);

    vec3 billboardUp = vec3(0, 1, 0);
    vec3 billboardRight = normalize(cross(viewDir, billboardUp)) * size;
    billboardUp = normalize(cross(billboardRight, viewDir)) * size;
    vec3 p1 = gs_in[0].worldPos - billboardRight - billboardUp;
    vec3 p2 = gs_in[0].worldPos + billboardRight - billboardUp;
    vec3 p3 = gs_in[0].worldPos - billboardRight + billboardUp;
    vec3 p4 = gs_in[0].worldPos + billboardRight + billboardUp;

    gl_Position = projection * view * vec4(p1, 1.0);
    TexCoords = vec2(1.0, 1.0);
    EmitVertex();

    gl_Position = projection * view * vec4(p2, 1.0);
    TexCoords = vec2(0.0, 1.0);
    EmitVertex();

    gl_Position = projection * view * vec4(p3, 1.0);
    TexCoords = vec2(1.0, 0.0);
    EmitVertex();

    gl_Position = projection * view * vec4(p4, 1.0);
    TexCoords = vec2(0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}

#region Fragment
#version 450 core
out vec4 FragColor;

in vec2 TexCoords;
in float texLayerIndex;
uniform sampler2DArray cloudArray;

void main() {
    FragColor = texture(cloudArray, vec3(TexCoords, texLayerIndex));
    if (FragColor.a < 0.5)
        discard;
    FragColor = vec4(FragColor.xyz, FragColor.w * 0.75);
}
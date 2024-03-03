#region Vertex

#version 440 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

out DATA_VERTEX {
    vec2 TexCoords;
    vec3 Normal;
} data_out;

void main()
{
    data_out.TexCoords = aTexCoords;
    data_out.Normal    = aNormal;
    gl_Position        = vec4(aPos, 1.0);
}

#region TesselationControl

#version 440 core

layout (vertices = 3) out;

in DATA_VERTEX {
    vec2 TexCoords;
    vec3 Normal;
} data_in[];

out DATA_TC {
    vec2 TexCoords;
    vec3 Normal;
} data_out[];

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    if(gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 1;
        gl_TessLevelOuter[1] = 1;
        gl_TessLevelOuter[2] = 1;
        gl_TessLevelInner[0] = 1;
    }

    data_out[gl_InvocationID].TexCoords = data_in[gl_InvocationID].TexCoords;
    data_out[gl_InvocationID].Normal = data_in[gl_InvocationID].Normal;
    gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
}

#region TesselationEvaluation

#version 440 core

layout (triangles, equal_spacing, ccw) in;

in DATA_TC {
    vec2 TexCoords;
    vec3 Normal;
} data_in[];

out DATA_TE {
    vec2 TexCoords;
    vec3 Normal;
} data_out;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
    return (gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2);
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return (gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2);
}

void main()
{
    vec3 pos0 = gl_in[0].gl_Position.xyz;
    vec3 pos1 = gl_in[1].gl_Position.xyz;
    vec3 pos2 = gl_in[2].gl_Position.xyz;

    vec2 uv0 = data_in[0].TexCoords;
    vec2 uv1 = data_in[1].TexCoords;
    vec2 uv2 = data_in[2].TexCoords;

    vec3 normal0 = data_in[0].Normal;
    vec3 normal1 = data_in[1].Normal;
    vec3 normal2 = data_in[2].Normal;

    gl_Position = vec4(interpolate3D(pos0, pos1, pos2), 1.0);
    data_out.TexCoords = interpolate2D(uv0, uv1, uv2);
    data_out.Normal = interpolate3D(normal0, normal1, normal2);
}

#region Geometry

#version 440 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in DATA_TE {
    vec2 TexCoords;
    vec3 Normal;
} data_in[];

out DATA_GS {
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPos;
    mat3 TBN;
} data_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vec3 edge0 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 edge1 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;

    vec2 deltaUV0 = data_in[1].TexCoords - data_in[0].TexCoords;
    vec2 deltaUV1 = data_in[2].TexCoords - data_in[0].TexCoords;

    float invDet = 1.0f / (deltaUV0.x * deltaUV1.y - deltaUV1.x * deltaUV0.y);

    vec3 tangent = vec3(invDet * (deltaUV1.y * edge0 - deltaUV0.y * edge1));

    vec3 T = normalize(vec3(model * vec4(tangent, 0.0f)));
    vec3 N = normalize(vec3(model * vec4(data_in[0].Normal, 0.0f)));

    // re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);

    // then retrieve perpendicular vector B with the cross product of T and N
    vec3 B = cross(N, T);

    mat3 TBNMat = transpose(mat3(T, B, N));

    vec3 Normal = normalize(cross(edge0, edge1));

    for(int i = 0; i < 3; i++)
    {
		data_out.FragPos = vec3(model * gl_in[i].gl_Position);
        gl_Position = projection * view * model * gl_in[i].gl_Position;
        data_out.TexCoords = data_in[i].TexCoords;
        data_out.Normal = Normal;
        data_out.TBN = TBNMat;
        EmitVertex();
    }
    
    EndPrimitive();
}

#region Fragment

#version 440 core

in DATA_GS {
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPos;
    mat3 TBN;
} data_in;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormalMap;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gNormal;
layout (location = 4) out vec3 gTBN1;
layout (location = 5) out vec3 gTBN2;
layout (location = 6) out vec3 gTBN3;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;

void main()
{   
    // G-buffer outputs
    gPosition = vec4(data_in.FragPos, 1.0);

    // Applying normal mapping
    gNormalMap = vec4(texture(normalTexture, data_in.TexCoords).rgb, 0.0f) * 2.0 - 1.0;
    gNormal = data_in.Normal;

    // Albedo and specular from textures
    vec3 albedo = texture(diffuseTexture, data_in.TexCoords).rgb;
    float specular = texture(specularTexture, data_in.TexCoords).r;
    gAlbedoSpec = vec4(albedo, specular);

    gTBN1 = vec3(data_in.TBN[0]);
    gTBN2 = vec3(data_in.TBN[1]);
    gTBN3 = vec3(data_in.TBN[2]);
}
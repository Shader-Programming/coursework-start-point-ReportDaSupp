#region Vertex

#version 440 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

out DATA_VERTEX {
    vec2 TexCoords;
    vec3 Normal;
    vec4 PosInWS;
    vec4 PosInSSP;
    vec4 PosInCSP;
} data_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    data_out.TexCoords = aTexCoords;
    data_out.Normal    = aNormal;
    
    data_out.PosInWS   = model * vec4(aPos, 1.0);
    data_out.PosInSSP  = view * data_out.PosInWS;
    data_out.PosInCSP  = projection * data_out.PosInSSP;
    gl_Position        = data_out.PosInWS;
}

#region TesselationControl

#version 440 core

layout (vertices = 3) out;

in DATA_VERTEX {
    vec2 TexCoords;
    vec3 Normal;
    vec4 PosInWS;
    vec4 PosInSSP;
    vec4 PosInCSP;
} data_in[];

out DATA_TC {
    vec2 TexCoords;
    vec3 Normal;
    vec4 PosInWS;
    vec4 PosInSSP;
    vec4 PosInCSP;
} data_out[];

uniform float tessLevel;
uniform vec3 viewPos;

void main()
{
    if(gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
        gl_TessLevelInner[0] = tessLevel;
    }
    
    data_out[gl_InvocationID].TexCoords = data_in[gl_InvocationID].TexCoords;
    data_out[gl_InvocationID].Normal = data_in[gl_InvocationID].Normal;
    data_out[gl_InvocationID].PosInWS = data_in[gl_InvocationID].PosInWS;
    data_out[gl_InvocationID].PosInSSP = data_in[gl_InvocationID].PosInSSP;
    data_out[gl_InvocationID].PosInCSP = data_in[gl_InvocationID].PosInCSP;
    gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
}

#region TesselationEvaluation

#version 440 core

layout (triangles, equal_spacing, ccw) in;

in DATA_TC {
    vec2 TexCoords;
    vec3 Normal;
    vec4 PosInWS;
    vec4 PosInSSP;
    vec4 PosInCSP;
} data_in[];

out DATA_TE {
    vec2 TexCoords;
    vec3 Normal;
    vec4 PosInWS;
    vec4 PosInSSP;
    vec4 PosInCSP;
} data_out;

uniform sampler2D heightTexture;

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

    vec4 posinWS0 = data_in[0].PosInWS;
    vec4 posinWS1 = data_in[1].PosInWS;
    vec4 posinWS2 = data_in[2].PosInWS;

    vec4 posinSSP0 = data_in[0].PosInSSP;
    vec4 posinSSP1 = data_in[1].PosInSSP;
    vec4 posinSSP2 = data_in[2].PosInSSP;

    vec4 posinCSP0 = data_in[0].PosInCSP;
    vec4 posinCSP1 = data_in[1].PosInCSP;
    vec4 posinCSP2 = data_in[2].PosInCSP;

    gl_Position = vec4(interpolate3D(pos0, pos1, pos2), 1.0);
    data_out.TexCoords = interpolate2D(uv0, uv1, uv2);
    data_out.Normal = interpolate3D(normal0, normal1, normal2);
    data_out.PosInWS = vec4(interpolate3D(posinWS0.xyz, posinWS1.xyz, posinWS2.xyz),1.0);
    data_out.PosInSSP = vec4(interpolate3D(posinSSP0.xyz, posinSSP1.xyz, posinSSP2.xyz),1.0);
    data_out.PosInCSP = vec4(interpolate3D(posinCSP0.xyz, posinCSP1.xyz, posinCSP2.xyz),1.0);

    //float height = texture(heightTexture, interpolate2D(data_in[0].TexCoords, data_in[1].TexCoords, data_in[2].TexCoords)).r * 2 - 1;
    //pos.y += height * 2.f;
}

#region Geometry

#version 440 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in DATA_TE {
    vec2 TexCoords;
    vec3 Normal;
    vec4 PosInWS;
    vec4 PosInSSP;
    vec4 PosInCSP;
} data_in[];

out DATA_GS {
    vec2 TexCoords;
    vec3 Normal;
    vec4 PosInWS;
    vec4 PosInSSP;
    vec4 PosInCSP;
} data_out;

void main()
{
    vec3 edge0 = data_in[1].PosInWS.xyz - data_in[0].PosInWS.xyz;
    vec3 edge1 = data_in[2].PosInWS.xyz - data_in[0].PosInWS.xyz;

    vec2 deltaUV0 = data_in[1].TexCoords - data_in[0].TexCoords;
    vec2 deltaUV1 = data_in[2].TexCoords - data_in[0].TexCoords;

    float invDet = 1.0f / (deltaUV0.x * deltaUV1.y - deltaUV1.x * deltaUV0.y);

    vec3 tangent = vec3(invDet * (deltaUV1.y * edge0 - deltaUV0.y * edge1));
    vec3 bitangent = vec3(invDet * (-deltaUV1.x * edge0 + deltaUV0.x * edge1));

    //vec3 T = normalize(vec3(data_in[0].Model * vec4(tangent, 0.0f)));
    //vec3 B = normalize(vec3(data_in[0].Model * vec4(bitangent, 0.0f)));
    //vec3 N = normalize(vec3(data_in[0].Model * vec4(cross(edge1, edge0), 0.0f)));
    //TBN = mat3(T, B, N);

    for(int i = 0; i < 3; i++)
    {
        data_out.PosInWS = data_in[i].PosInWS;
        data_out.PosInSSP = data_in[i].PosInSSP;
        data_out.PosInCSP = data_in[i].PosInCSP;
        data_out.TexCoords = data_in[i].TexCoords;
        data_out.Normal = data_in[i].Normal;
        EmitVertex();
    }
    
    EndPrimitive();
}

#region Fragment

#version 440 core

in DATA_GS {
    vec2 TexCoords;
    vec3 Normal;
    vec4 PosInWS;
    vec4 PosInSSP;
    vec4 PosInCSP;
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
    gPosition = data_in.PosInWS;

    // Applying normal mapping
    gNormalMap = vec4(texture(normalTexture, data_in.TexCoords).rgb, 0.0f);
    gNormal = data_in.Normal;

    // Albedo and specular from textures
    vec3 albedo = texture(diffuseTexture, data_in.TexCoords).rgb;
    float specular = texture(specularTexture, data_in.TexCoords).r;
    gAlbedoSpec = vec4(albedo, specular);

    //gTBN1 = vec3(TBN[0]);
    //gTBN2 = vec3(TBN[1]);
    //gTBN3 = vec3(TBN[2]);
}
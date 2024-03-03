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
uniform vec3 viewPos;

//float calculateScreenSpaceEdgeLength(vec3 vertexPositionWorldSpace, vec3 otherVertexPositionWorldSpace) {
//    vec4 vertexPositionClipSpace = projection * view * vec4(vertexPositionWorldSpace, 1.0);
//    vec4 otherVertexPositionClipSpace = projection * view * vec4(otherVertexPositionWorldSpace, 1.0);
//    
//    vec3 vertexPositionNDC = vertexPositionClipSpace.xyz / vertexPositionClipSpace.w;
//    vec3 otherVertexPositionNDC = otherVertexPositionClipSpace.xyz / otherVertexPositionClipSpace.w;
//   
//    float screenSpaceDistance = length(vertexPositionNDC.xy - otherVertexPositionNDC.xy);
//    return screenSpaceDistance;
//}

float getDistanceFromCamera(vec3 worldPos) {
    return length(viewPos - worldPos);
}

float calculateTessLevel(float distance) {

    const float maxDistance = 200.0; // Maximum distance for tessellation
    const float minTessLevel = 8.0; // Minimum tessellation level
    const float maxTessLevel = 64.0; // Maximum tessellation level
    
    float normalizedDistance = clamp(distance / maxDistance, 0.0, 1.0);
    
    float tessLevel = mix(maxTessLevel, minTessLevel, normalizedDistance);
    return tessLevel;
}

void main()
{
    // Screen Space Method
    //vec3 controlPointsWorld[3];
    //for (int i = 0; i < 3; ++i) {
    //    vec4 worldPos = model * gl_in[i].gl_Position;
    //    controlPointsWorld[i] = worldPos.xyz;
    //}

    //float avgEdgeLength = 0.0;
    //for (int i = 0; i < 3; ++i) {
    //    avgEdgeLength += calculateScreenSpaceEdgeLength(controlPointsWorld[i], controlPointsWorld[(i + 1) % 3]);
    //}
    //avgEdgeLength /= 3.0;
    //
    //float tessLevel = 1.0 / avgEdgeLength * 10.0; // Example scaling factor

    // View Distance Method
    vec4 worldPos = model * gl_in[gl_InvocationID].gl_Position;
    
    float distance = getDistanceFromCamera(worldPos.xyz);
    
    float tessLevel = calculateTessLevel(distance);

    if(gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
        gl_TessLevelInner[0] = tessLevel;
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
    vec3 Color;
} data_out;

vec3 lowColor = vec3(0.0, 0.0, 1.0);
vec3 midColor = vec3(0.0, 1.0, 0.0); 
vec3 highColor = vec3(1.0, 0.0, 0.0); 

float midHeight = 0.5; 
float highHeight = 1.0; 

uniform sampler2D HeightMapTex;
uniform sampler2D DuDvMapTex;

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

    gl_Position = vec4(interpolate3D(pos0, pos1, pos2), 1.0);
    data_out.TexCoords = interpolate2D(uv0, uv1, uv2);
    data_out.Normal = texture(HeightMapTex, interpolate2D(data_in[0].TexCoords, data_in[1].TexCoords, data_in[2].TexCoords)).rgb;

    float height = texture(HeightMapTex, data_out.TexCoords).a;
    gl_Position.y += (height * 2 - 1) * 10.f;

    if (height <= 0.4)
    {
        gl_Position.y -= (texture(DuDvMapTex, data_out.TexCoords).r) - 0.5;
        data_out.Color = vec3(0.1, 0.1, 0.87);
    }
    else if(height < midHeight) {
        float t = height / midHeight;
        data_out.Color = mix(lowColor, midColor, t);
    } 
    else {
        float t = (height - midHeight) / (highHeight - midHeight);
        data_out.Color = mix(midColor, highColor, t);
    }
    
}

#region Geometry

#version 440 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in DATA_TE {
    vec2 TexCoords;
    vec3 Normal;
    vec3 Color;
} data_in[];

out DATA_GS {
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPos;
    mat3 TBN;
    vec3 Color;
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
        data_out.Color = data_in[i].Color;
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
    vec3 Color;
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
uniform sampler2D HeightMapTex;

void main()
{   
    // G-buffer outputs
    gPosition = vec4(data_in.FragPos, 1.0);

    // Applying normal mapping
    gNormalMap = vec4(texture(HeightMapTex, data_in.TexCoords).rgb, 0.0f) * 2.0 - 1.0;
    gNormal = data_in.Normal;

    // Albedo and specular from textures
    //vec3 albedo = texture(diffuseTexture, data_in.TexCoords).rgb;
    vec3 albedo = data_in.Color;
    float specular = texture(specularTexture, data_in.TexCoords).r;
    gAlbedoSpec = vec4(albedo, specular);

    gTBN1 = vec3(data_in.TBN[0]);
    gTBN2 = vec3(data_in.TBN[1]);
    gTBN3 = vec3(data_in.TBN[2]);
}
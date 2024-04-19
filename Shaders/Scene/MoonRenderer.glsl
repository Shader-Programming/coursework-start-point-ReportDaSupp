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
uniform bool moon;
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

        if (displacement <= 0.5)
            displacement = displacement * 0.5 + 0.25;
        else
            displacement = 0.5 - (displacement * 0.25);
        
        float effectiveRadius = 0.93 + displacement * 0.15;
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

uniform vec3 cameraPos;
uniform samplerCube heightMap;
uniform sampler2DArray AlbedoArray;
uniform sampler2DArray NormalArray;

uniform vec3 lightPosition = vec3(0, 200, 200);
uniform vec3 lightColor = vec3(0.5, 0.5, 0.5);
vec3 ambientColor = vec3(0.2, 0.2, 0.2);

vec2 selectTriplanarUV(vec3 worldPos, vec3 normal) {
    // Normalize the position to ensure it is properly scaled
    worldPos = normalize(worldPos);

    // Determine the maximum component of the normal vector
    vec3 absNormal = abs(normal);
    float maxComponent = max(max(absNormal.x, absNormal.y), absNormal.z);

    vec2 uv;

    // Check which component is the dominant one and set UV coordinates accordingly
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

void main() {

    vec3 normal = texture(NormalArray, vec3(selectTriplanarUV(UV3, Normal), 3)).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    ambientColor = texture(AlbedoArray, vec3(selectTriplanarUV(UV3, Normal), 3)).rgb;

    vec3 lightDir = normalize(lightPosition - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lightColor;

    // Combine results
    vec3 color = ambientColor + diffuse + specular;
    FragColor = vec4(color, 1.0);
}
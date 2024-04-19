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

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform bool sphere;
uniform float elapsedTime;

uniform samplerCube heightMap;

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
        float wave = sin(normPosition.x * 20.0 + elapsedTime / 3) * 0.01;
        float displacement = 0.47 + wave;

        float effectiveRadius = 0.93 + displacement * 0.15;
        vec3 spherePos = normPosition * effectiveRadius;

        vec3 modelPosition = (vec4(spherePos, 1.0)).xyz;

        if (!sphere)
        {
            spherePos = tes_position[i] * 0.93 + displacement * 0.15;
            modelPosition = (vec4(spherePos, 1.0)).xyz;
        }

        vec3 perturbedNormal = calculateNormal(modelPosition, normPosition);

        FragPos = modelPosition;
        Normal = perturbedNormal;
        UV3 = normPosition;

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
uniform samplerCube skybox;  // Assuming a skybox for environment reflections
uniform float refractionIndex = 1.33; // Index of refraction for water

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() 
{
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-viewDir, Normal);
    vec3 refractDir = refract(-viewDir, Normal, 1.0 / refractionIndex);

    vec3 reflectedColor = texture(skybox, reflectDir).rgb;
    vec3 refractedColor = texture(skybox, refractDir).rgb;

    vec3 F0 = vec3(0.02);
    float fresnel = fresnelSchlick(max(dot(viewDir, Normal), 0.0), F0).x;
    
    vec3 color = mix(refractedColor, reflectedColor, fresnel);

    FragColor = vec4(color, 0.3);  // Semi-transparent water
}
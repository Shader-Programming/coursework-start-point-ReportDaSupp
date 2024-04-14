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
    vs_out.position = vec3(model * vec4(aPos, 1.0));
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

float tessLevel = 16;

void main() {
    tcs_out[gl_InvocationID].position = tcs_in[gl_InvocationID].position;
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
    
    tes_position = pos;
    gl_Position = model * vec4(pos, 1.0);
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
uniform bool water;
uniform float elapsedTime;

uniform samplerCube heightMap;

float radius = 1;

vec3 calculateNormal(vec3 pos, vec3 normPosition) {
    float eps = 0.01;  // Small value for finite difference approximation
    // Fetch heights around the position to compute gradient
    float heightCenter = texture(heightMap, normPosition).r;
    float heightX = texture(heightMap, normalize(normPosition + vec3(eps, 0, 0))).r;
    float heightY = texture(heightMap, normalize(normPosition + vec3(0, eps, 0))).r;
    float heightZ = texture(heightMap, normalize(normPosition + vec3(0, 0, eps))).r;

    // Compute gradient and use it to perturb the normal
    vec3 gradient = vec3(heightX - heightCenter, heightY - heightCenter, heightZ - heightCenter) / eps;
    vec3 perturbedNormal = normalize(normPosition - gradient);

    return perturbedNormal;
}

void main() {
    vec3 normals[3];  // Normals for each vertex

    float rotationSpeed = -0.03; 
    float angle = elapsedTime * rotationSpeed;
    mat3 rotationMatrix = mat3(
        cos(angle), 0.0, sin(angle),
        0.0, 1.0, 0.0,
        -sin(angle), 0.0, cos(angle)
    );

    for (int i = 0; i < 3; i++) {
        vec3 normPosition = normalize(tes_position[i]);
        float displacement = water ? 0.0 : texture(heightMap, normPosition).r; 
        float effectiveRadius = water ? 1 : (0.93 + displacement * 0.15);
        vec3 spherePos = normPosition * effectiveRadius * 150;

        vec3 rotatedPosition = rotationMatrix * spherePos;
        vec3 rotatedNormal = calculateNormal(rotatedPosition, normPosition);

        FragPos = rotatedPosition;
        Normal = rotatedNormal;
        UV3 = normPosition;

        gl_Position = projection * view * vec4(rotatedPosition, 1.0);
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

uniform bool water;
uniform vec3 cameraPos;

uniform sampler2D sandTexture;
uniform sampler2D snowTexture;
uniform sampler2D forestTexture;
uniform sampler2D grassTexture;
uniform sampler2D beachTexture;

uniform samplerCube heightMap;
uniform samplerCube tempMap;
uniform samplerCube precipMap;

vec3 getBiomeColor(float height, float temperature, float precipitation) {
    vec3 biomeColor = vec3(0.5, 0.5, 0.5);
    vec3 sandColor = vec3(0.75, 0.69, 0.5); // Beach sand
    vec3 dirtColor = vec3(0.36, 0.25, 0.20); // Underwater dirt
    vec3 grassColor = vec3(0.43, 0.32, 0.24); // Grasslands
    vec3 rockColor = vec3(0.5, 0.5, 0.5); // Mountainous areas
    vec3 snowColor = vec3(0.9, 0.9, 0.9); // Snow at high altitudes or cold temperatures
    vec3 forestColor = vec3(0.28, 0.36, 0.03); // Forests in wet areas

    vec3 waterBaseColor = mix(dirtColor, sandColor, smoothstep(0.4, 0.48, height));

    if (height < 0.48) {
        float depthFactor = (0.48 - height) / 0.48;
        waterBaseColor = mix(waterBaseColor, dirtColor * 0.5, depthFactor);
        biomeColor = mix(waterBaseColor, sandColor, smoothstep(0.43, 0.45, height));
    } else {
        biomeColor = mix(grassColor, rockColor, smoothstep(0.6, 0.75, height));
        
        if (temperature < 0.1) {
            biomeColor = mix(biomeColor, snowColor, smoothstep(0.0, 0.1, temperature));
        }

        if (precipitation > 0.15) {
            biomeColor = mix(biomeColor, forestColor, smoothstep(0.15, 0.25, precipitation));
        }

        float desertFactor = smoothstep(0.75, 0.85, temperature) * (1.0 - smoothstep(0.0, 0.1, precipitation));

        biomeColor = mix(biomeColor, sandColor, desertFactor);
    }

    return biomeColor;
}

void main() {
    float height = texture(heightMap, UV3).r;
    float temperature = texture(tempMap, UV3).r;
    float precipitation = texture(precipMap, UV3).r;

    vec3 biomeColor = getBiomeColor(height, temperature, precipitation);

    if (!water)
    {
        vec3 lightDir = normalize(vec3(0.5, 1.0, -0.5));
        float diff = max(dot(Normal, lightDir), 0.0);
        vec3 diffuse = diff * vec3(biomeColor);
        vec3 ambient = 0.3 * vec3(biomeColor); 
        FragColor = vec4(diffuse + ambient, 1.0);
        FragColor = mix(FragColor, vec4(temperature, precipitation, 1-temperature, 1.0), 0.17);
        //FragColor = vec4(Normal, 1.0);
    }
    else
    {
        FragColor =  vec4(0.1, 0.2, 0.7, 0.3);
    }
}
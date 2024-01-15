#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 UV ;
out vec3 normal ;
out vec3 posInWS ;


void main()
{  
    posInWS = (model*vec4(aPos,1.0)).xyz; 
    gl_Position = projection * view * vec4(posInWS,1.0);
    UV = aUV ;
    normal = vec3(0.0,1.0,0.0) ;  // for now plane is flat so all normals pointing up
    
}
#region Vertex

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

#region Fragment

#version 410 core

///////////////////////// INS OUTS
out vec4 FragColor;

in vec2 UV ;
in vec3 normal ;
in vec3 posInWS ;


///////////////////////// UNIFORMS
uniform vec3 lightDirection ;
uniform vec3 floorCol ;
uniform vec3 viewPos ;

///////////////////////// FUNCTIONS
vec3 getDirectionalLight() ;

///////////////////////// VARS
// constants for now, could be added to imGui?
float ambientFactor = 0.5 ;
float shine = 16.0f ;
float specularStrength = 0.3f ;
vec3 lightColour = vec3(1.0f) ;
vec3 viewDir ;
vec3 n ;
vec3 lightDir ;

///////////////////////// MAIN

void main()
{    	
    n = normalize(normal) ;
    viewDir = normalize(viewPos - posInWS);
    lightDir = -normalize(lightDirection) ;
    lightDir.y = max(lightDir.y, 0.0) ; // Gui slider goes from [-1,1], direction light can only go from [0,1]


    vec3 result = getDirectionalLight() ;
    FragColor = vec4(result, 1.0f);   
}


///////////////////////// DIRECTIONAL LIGHT

vec3 getDirectionalLight() { 
   

   // ambient
  vec3 ambient = floorCol * lightColour * ambientFactor;

  //diffuse 
  float diffuseFactor = dot(n,lightDir) ;
  diffuseFactor = max(diffuseFactor, 0.0f) ;
  vec3 diffuse = floorCol * lightColour * diffuseFactor;


  // Blinn Phong specular
  vec3 H = normalize(lightDir + viewDir ) ;  
  float specLevel = dot(n, H) ;   
  specLevel = max(specLevel, 0.0) ; 
  specLevel = pow(specLevel, shine) ; 
  vec3 specular = lightColour*specLevel* specularStrength;

  return ambient + specular + diffuse;

}
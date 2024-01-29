#version 440 core

uniform sampler2D DownSampleTexture;
uniform vec2 Resolution;

in vec2 TexCoords;
layout (location = 0) out vec3 downsample;

void main()
{
    vec2 srcTexelSize = 1.0 / Resolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    vec3 a = texture(DownSampleTexture, vec2(TexCoords.x - 2*x, TexCoords.y + 2*y)).rgb;
    vec3 b = texture(DownSampleTexture, vec2(TexCoords.x,       TexCoords.y + 2*y)).rgb;
    vec3 c = texture(DownSampleTexture, vec2(TexCoords.x + 2*x, TexCoords.y + 2*y)).rgb;

    vec3 d = texture(DownSampleTexture, vec2(TexCoords.x - 2*x, TexCoords.y)).rgb;
    vec3 e = texture(DownSampleTexture, vec2(TexCoords.x,       TexCoords.y)).rgb;
    vec3 f = texture(DownSampleTexture, vec2(TexCoords.x + 2*x, TexCoords.y)).rgb;

    vec3 g = texture(DownSampleTexture, vec2(TexCoords.x - 2*x, TexCoords.y - 2*y)).rgb;
    vec3 h = texture(DownSampleTexture, vec2(TexCoords.x,       TexCoords.y - 2*y)).rgb;
    vec3 i = texture(DownSampleTexture, vec2(TexCoords.x + 2*x, TexCoords.y - 2*y)).rgb;

    vec3 j = texture(DownSampleTexture, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 k = texture(DownSampleTexture, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;
    vec3 l = texture(DownSampleTexture, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 m = texture(DownSampleTexture, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;

    downsample = e*0.125;
    downsample += (a+c+g+i)*0.03125;
    downsample += (b+d+f+h)*0.0625;
    downsample += (j+k+l+m)*0.125;
}
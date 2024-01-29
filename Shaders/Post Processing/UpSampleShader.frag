#version 440 core

uniform sampler2D UpSampleTexture;
uniform float filterRadius;

in vec2 TexCoords;
layout (location = 0) out vec3 upsample;

void main()
{
    float x = filterRadius;
    float y = filterRadius;

    vec3 a = texture(UpSampleTexture, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 b = texture(UpSampleTexture, vec2(TexCoords.x,     TexCoords.y + y)).rgb;
    vec3 c = texture(UpSampleTexture, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;

    vec3 d = texture(UpSampleTexture, vec2(TexCoords.x - x, TexCoords.y)).rgb;
    vec3 e = texture(UpSampleTexture, vec2(TexCoords.x,     TexCoords.y)).rgb;
    vec3 f = texture(UpSampleTexture, vec2(TexCoords.x + x, TexCoords.y)).rgb;

    vec3 g = texture(UpSampleTexture, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 h = texture(UpSampleTexture, vec2(TexCoords.x,     TexCoords.y - y)).rgb;
    vec3 i = texture(UpSampleTexture, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;

    upsample = e*4.0;
    upsample += (b+d+f+h)*2.0;
    upsample += (a+c+g+i);
    upsample *= 1.0 / 16.0;
}
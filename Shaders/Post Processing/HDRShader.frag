#version 440 core

uniform sampler2D ColorAttachment;
uniform sampler2D BloomAttachment;

uniform bool eBloom;
uniform bool eTone;
uniform bool eInverse;
uniform bool eGrayscale;
uniform bool eGamma;

out vec4 FragColor;

in vec2 TexCoords;

vec3 ACESFilm(vec3 x)
{
float a = 2.51f;
float b = 0.03f;
float c = 2.43f;
float d = 0.59f;
float e = 0.14f;
return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec3 toneMapReinhard(vec3 color) {
    color = color / (color + vec3(1.0));
    return color;
}

vec3 Grayscale(vec3 x)
{
	vec3 color = x;
	float avg = (color.x + color.y + color.z) / 3;
	return vec3(avg, avg, avg);
};

vec3 Inverse(vec3 x)
{
	vec3 color = x;

	return vec3(1 - color.x, 1 - color.y, 1 - color.z);
};

vec3 Gamma(vec3 x)
{
	vec3 color = pow(x, vec3(1.0/2.2));
	return color;
}

void main()
{
	vec3 result = texture(ColorAttachment, TexCoords).rgb;

	if (eBloom)
	{ result = result + texture(BloomAttachment, TexCoords).rgb * 0.5; }

	if (eTone)
	{ result = toneMapReinhard(result); }

	if (eGamma)
	{ result = Gamma(result); }

	if (eInverse)
	{ result = Inverse(result); }

	if (eGrayscale)
	{ result = Grayscale(result); }

	FragColor = vec4(result, 1.0);
}
#version 330 core
out vec4 FragColor;

in VSOut
{
	vec3 Normal;
	vec2 TexCoord;
	vec3 FragPos;
} psin;

uniform vec3 camPos;
uniform samplerCube skybox;
uniform float refractionIndex;

void main()
{
	vec3 camToFrag = normalize( psin.FragPos - camPos );
	float refractionRatio = 1.0 / refractionIndex;
	vec3 refractVec = refract(camToFrag, normalize(psin.Normal), refractionRatio);
	FragColor = vec4(texture(skybox, refractVec).rgb, 1.0);
}
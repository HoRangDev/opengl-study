#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoord;
in vec3 FragPos;

uniform vec3 camPos;
uniform samplerCube skybox;
uniform float refractionIndex;

void main()
{
	vec3 camToFrag = normalize( FragPos - camPos );
	float refractionRatio = 1.0 / refractionIndex;
	vec3 refractVec = refract(camToFrag, normalize(Normal), refractionRatio);
	FragColor = vec4(texture(skybox, refractVec).rgb, 1.0);
}
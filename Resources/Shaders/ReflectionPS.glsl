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

void main()
{
	vec3 camToFrag = normalize( psin.FragPos - camPos );
	vec3 reflectVec = reflect(camToFrag, normalize(psin.Normal));
	FragColor = vec4(texture(skybox, reflectVec).rgb, 1.0);
}
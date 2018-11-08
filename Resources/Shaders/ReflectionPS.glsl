#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoord;
in vec3 FragPos;

uniform vec3 camPos;
uniform samplerCube skybox;

void main()
{
	vec3 camToFrag = normalize( FragPos - camPos );
	vec3 reflectVec = reflect(camToFrag, normalize(Normal));
	FragColor = vec4(texture(skybox, reflectVec).rgb, 1.0);
}
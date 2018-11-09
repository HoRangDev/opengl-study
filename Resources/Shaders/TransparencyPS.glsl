#version 330 core

out vec4 FragColor;

in VSOut
{
	vec3 Normal;
	vec2 TexCoord;
	vec3 FragPos;
} psin;
uniform sampler2D texture1;

void main( )
{
	FragColor = texture(texture1, psin.TexCoord);
	if (FragColor.a < 0.1)
	{
		discard;
	}
}
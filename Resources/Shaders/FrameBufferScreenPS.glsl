#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
	FragColor =  texture(screenTexture, TexCoords);
	float avg = ( FragColor.r + FragColor.g + FragColor.b ) / 3.0;
	FragColor = vec4(avg, avg, avg, 1.0f);
}
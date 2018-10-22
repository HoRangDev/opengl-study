#version 330 core
out vec4 FragColor;
uniform vec3 ourColor;
in vec3 colorOut;
in vec2 texCoordOut;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	FragColor = mix(texture(texture1, texCoordOut), texture(texture2, texCoordOut), 0.2);
	//FragColor = texture(_texture1, texCoordOut);
}

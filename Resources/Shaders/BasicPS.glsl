#version 330 core
out vec4 FragColor;
uniform vec3 ourColor;
in vec3 colorOut;
in vec2 texCoordOut;

uniform sampler2D ourTexture;

void main()
{
	FragColor = vec4(clamp(colorOut.x + ourColor.x, 0.0, 1.0),	
	clamp(colorOut.y + ourColor.y, 0.0, 1.0),
	clamp(colorOut.z + ourColor.z, 0.0, 1.0), 1.0);
	FragColor = texture(ourTexture, texCoordOut);
}

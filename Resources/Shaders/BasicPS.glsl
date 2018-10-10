#version 330 core
out vec4 FragColor;

in vec3 colorOut;

void main()
{
	FragColor = vec4(colorOut.x, colorOut.y, colorOut.z, 1.0);
}

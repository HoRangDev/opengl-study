#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1/300.0;

void main()
{
	vec2 offsets[9] = vec2[](
		vec2(-offset, offset),
		vec2(0.0, offset),
		vec2(offset, offset),
		vec2(-offset, 0.0),
		vec2( 0.0, 0.0 ),
		vec2( offset, 0.0),
		vec2( -offset, -offset),
		vec2(0.0, -offset),
		vec2(offset, -offset)
	);

	float kernel[9] = float[](
	1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
	1.0 / 16.0, 4.0 / 16.0, 1.0 / 16.0,
	1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0
	);

	vec3 sampleTex[9];
	for (int idx = 0; idx < 9; ++idx)
	{
		sampleTex[idx] = vec3(texture(screenTexture, (TexCoords.st + offsets[idx])));
	}

	vec3 col = vec3(0.0);
	for ( int idx = 0; idx < 9; ++idx )
	{
		col += (sampleTex[idx] * kernel[idx]);
	}

	FragColor =  vec4(col, 1.0);
}
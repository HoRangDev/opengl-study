#version 330 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D image;

uniform bool horizontal;
uniform float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 tex_offset = 1.0 / textureSize(image, 0);
    vec3 result = texture(image, texCoords).rgb * weights[0];
    if (horizontal)
    {
        for (int idx = 1; idx < 5; ++idx)
        {
            result += texture(image, texCoords + vec2(tex_offset.x * idx, 0.0)).rgb * weights[idx];
            result += texture(image, texCoords - vec2(tex_offset.x * idx, 0.0)).rgb * weights[idx];
        }
    }
    else
    {
        for (int idx = 1; idx < 5; ++idx)
        {
            result += texture(image, texCoords + vec2(0.0, tex_offset.y * idx)).rgb * weights[idx];
            result += texture(image, texCoords - vec2(0.0, tex_offset.y * idx)).rgb * weights[idx];
        }
    }

    FragColor = vec4(result, 1.0);
}
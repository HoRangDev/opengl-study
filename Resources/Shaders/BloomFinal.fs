#version 330 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure;

void main()
{
    FragColor = vec4(texture(bloomBlur, texCoords).rgb, 1.0);
    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, texCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, texCoords).rgb;
    //hdrColor += bloomColor;

    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    result = pow(result, vec3(1.0/gamma));
    FragColor = vec4(result, 1.0);
}
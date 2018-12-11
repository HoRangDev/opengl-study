#version 330 core
out vec4 FragColor;

in VS_OUT
{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
}fsin;

struct Light
{
    vec3 position;
    vec3 color;
};

uniform Light lights[3];
uniform vec3 viewPos;

uniform sampler2D diffuseMap;

void main()
{
    vec3 normal = normalize(fsin.normal);
    vec3 color = texture(diffuseMap, fsin.texCoords).rgb;

    float ambient = 0.1f;
    vec3 ambientColor = ambient * color;

    vec3 lightRes = vec3(0.0);
    for(int idx = 0; idx < 16; ++idx)
    {
        vec3 lightPos = lights[idx].position;

        vec3 fragToLight = normalize(lightPos - fsin.fragPos);
        float diffuse = max(0.0, dot(fragToLight, normal));
        vec3 diffuseColor = diffuse * color * lights[idx].color;

        vec3 res = diffuseColor;
        float dist = length(lightPos - fsin.fragPos);
        float attenutation = 1.0 / (dist * dist);
        res *= attenutation;
        lightRes += res;
    }

    FragColor = vec4(lightRes + ambientColor, 1.0);
}
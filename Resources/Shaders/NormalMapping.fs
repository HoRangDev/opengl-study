#version 330 core

out vec4 FragColor;

in VSOut
{
    vec3 position;
    vec2 texCoord;
    mat3 TBN;
}fsin;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

void main()
{
    vec3 fragToLight = normalize(lightPos - fsin.position);
    vec3 normal = texture(normalMap, fsin.texCoord).rgb;
    normal = normalize((normal * 2.0) - 1.0);
    normal = normalize(fsin.TBN * normal);

    vec3 color = pow(texture(diffuseMap, fsin.texCoord).rgb, vec3(2.4));
    float diffuse = max(0.0, dot(fragToLight, normal));
    vec3 diffuseColor = color * diffuse;

    float ambient = 0.1;
    vec3 ambientColor = color * ambient;

    vec3 fragToView = normalize(viewPos - fsin.position);
    vec3 reflectLight = normalize(reflect(-fragToLight, normal));
    vec3 halfWay = normalize(fragToLight + fragToView);
    float specular = pow(max(0, dot(halfWay, reflectLight)), 32.0);
    vec3 specularColor = vec3(0.2) * specular; // white

    FragColor = vec4(ambientColor + diffuseColor + specularColor, 1.0);
    FragColor = pow(FragColor, vec4(1/2.4));
}
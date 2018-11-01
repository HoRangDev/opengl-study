#version 330 core
struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

in vec3 Normal;
in vec3 FragPos;

void main()
{
	vec3 ambient = light.ambient * material.ambient;

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos); // Frag to light direction
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * material.diffuse * light.diffuse;

	vec3 viewDir = normalize(viewPos - FragPos); // Frag to viewer
	vec3 reflectDir = reflect( -lightDir, norm );
	float spec = pow( max(dot(viewDir, reflectDir), 0.0 ), material.shininess );
	vec3 specular = spec * material.specular * light.specular;

	vec3 result = ( ambient + diffuse + specular );
	FragColor = vec4(result, 1.0);
}
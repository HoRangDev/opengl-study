#version 330 core
struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	sampler2D emission;
	float shininess;
};

struct Light
{
	vec3 position;
	vec3 direction;
	float cutOff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;
uniform vec2 emissionTexOffset;

in vec3 Normal;
in vec2 TexCoord;
in vec3 FragPos;

void main()
{
	vec3 norm = normalize(Normal);
	vec3 lightDir = (light.position - FragPos); // Frag to light direction
	float dist = length(lightDir);
	float att = 1.0f / ( light.constant + ( light.linear * dist ) + ( light.quadratic * dist * dist ) );
	lightDir = normalize(lightDir);

	float theta = dot(lightDir, normalize(-light.direction));

	if ( theta > light.cutOff )
	{
		float diff = max(dot(norm, lightDir), 0.0);

		vec3 viewDir = normalize(viewPos - FragPos); // Frag to viewer
		vec3 reflectDir = reflect( -lightDir, norm );
		float spec = pow( max(dot(viewDir, reflectDir), 0.0 ), material.shininess );

		vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
		vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, TexCoord));
		vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoord));

		ambient *= att;
		diffuse *= att;
		specular *= att;

		vec3 result = ( ambient + diffuse + specular + vec3(texture(material.emission, TexCoord + emissionTexOffset) ) );
		FragColor = vec4(result, 1.0);	
	}
	else
	{
		FragColor = vec4(light.ambient * vec3(texture(material.diffuse, TexCoord)), 1.0f);		
	}
}
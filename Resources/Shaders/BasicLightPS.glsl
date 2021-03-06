#version 330 core
struct Material
{
	sampler2D texture_diffuse0;
	sampler2D texture_specular0;
	sampler2D texture_ambient0; // Use as reflection map
	float shininess;
	vec3 baseColor;
};

struct DirLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight
{
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

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
uniform samplerCube skybox;

uniform DirLight dirLight;

#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

in VSOut
{
	vec3 Normal;
	vec2 TexCoord;
	vec3 FragPos;
} psin;

// normal, viewDir => Always be normalized
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);
	vec3 halfway = normalize(lightDir + viewDir);

	// Diffuse
	float diff = max(dot(lightDir, normal), 0.0);

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	//float spec = pow(max(dot(normalize(viewDir), reflectDir), 0.0), material.shininess); // Phong
	float spec = pow(max(dot(normal, halfway), 0.0), material.shininess); // Blinn-Phong

	vec3 ambient = light.ambient * (material.baseColor + vec3(texture(material.texture_diffuse0, psin.TexCoord)));
	vec3 diffuse = light.diffuse * (material.baseColor + vec3(texture(material.texture_diffuse0, psin.TexCoord))) * diff;
	vec3 specular = light.specular * (material.baseColor + vec3(texture(material.texture_specular0, psin.TexCoord))) * spec;

	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = light.position - fragPos;
	float dist = length(lightDir);
	lightDir = normalize(lightDir);

	DirLight dirLight;
	dirLight.direction = -lightDir; // point to light -> light to point
	dirLight.ambient = light.ambient;
	dirLight.diffuse = light.diffuse;
	dirLight.specular = light.specular;

	vec3 result = CalcDirLight(dirLight, normal, viewDir);

	float att = 1.0 / ( light.constant + ( light.linear * dist ) + ( light.quadratic * dist * dist ) );
	result *= att; // Apply attenutation

	return result;
}

vec3 CalcSpotLight(SpotLight light, vec3 fragPos, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos); // fragment to light

	float theta = dot(lightDir, normalize(-light.direction)); // light.direction = light to aiming point / lightDir = fragment to light
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff)/epsilon, 0.0, 1.0);
	// if |theta| getting larger cos(theta)->0
	// |cutOff| < |outerCutOff| => cos(cutOff) > cos(outerCutOff) => epsilon > 0
	// |theta| < |cutOff| => cos(theta) > cos(cutOff) > cos(outerCutOff) => cos(theta) > epsilon > 0
	// |theta| > |cutOff| ^ |theta| < |outerCutOff| => cos(theta) < cos(cutOff) , cos(theta) > cos(outerCutOff) => cos(outerCutOff) < cos(theta) < cos(cutOff) => 0 < cos(theta) < epsilon
	// |theta| > |outerCutOff| => cos(theta) < cos(outerCutOff), cos(theta) < cos(outerCutOff) < cos(cutOff) => cos(theta) < 0 < epsilon
	
	PointLight pLight;
	pLight.position = light.position;
	pLight.constant = light.constant;
	pLight.linear = light.linear;
	pLight.quadratic = light.quadratic;
	pLight.ambient = light.ambient;
	pLight.diffuse = light.diffuse;
	pLight.specular = light.specular;

	vec3 result = CalcPointLight(pLight, fragPos, normal, viewDir);
	result *= intensity; // Apply smooth cut-off

	return result;
}

void main()
{
	vec3 result;
	vec3 norm = normalize(psin.Normal);
	vec3 viewDir = normalize(viewPos - psin.FragPos);

	result += CalcDirLight(dirLight, norm, viewDir);

	for ( int idx = 0; idx < NR_POINT_LIGHTS; ++idx )
	{
		result += CalcPointLight(pointLights[idx], psin.FragPos, norm, viewDir);
	}
	
	//result += CalcSpotLight(spotLight, FragPos, norm, viewDir);

	vec3 reflectVec = reflect(-viewDir, norm);
	vec3 reflectColor = texture(skybox, reflectVec).rgb;
	reflectColor = reflectColor * texture(material.texture_ambient0, psin.TexCoord).rgb;
	FragColor = vec4(result, 1.0f);
}
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal)
{
    // 빛이 바라보는 관점에서의 프래그먼트가 원래
    // Shadow Depth 패스에서 계산되었던 가장 가까운 프래그먼트의 Depth 보다 큰지 작은지 확인한 후에.
    // 만약 더 멀리있다면 Shadow를 적용 아니라면 적용X
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
    {
        return 0.0;
    }

    float currentDepth = projCoords.z;

     // Shadow bias => shadow map resolution 으로 생기는 shadow acne 현상을 방지하기위해
     // https://a.disquscdn.com/uploads/mediaembed/images/3808/4980/original.jpg?w=800&h
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = 1.0/textureSize(shadowMap, 0);
    for( int x = -1; x <= 1; ++x)
    {
        for (int y=-1; y <= 1; ++y)
        {
            // uv-coordinate
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth-bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

void main()
{
    // Blinn-Phong Shading model
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3f);
    vec3 ambient = 0.3 * color;
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    // diffuse
    float diff = max(dot(lightDir, normal), 0.0); // Lambertian Diffuse
    vec3 diffuse = diff * lightColor;

    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;

    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, lightDir, normal);
    // 만약 Shadow 가 생긴다면 ambient 항만 적용!
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse+specular)) * color;
    FragColor = vec4(lighting, 1.0);    
}
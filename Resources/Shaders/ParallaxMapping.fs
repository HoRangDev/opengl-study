#version 330 core
out vec4 FragColor;

in VS_OUT
{
    vec3 fragPosition;
    vec2 texCoords;
    vec3 tangentLightPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
}fsin;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform float heightScale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0,0.0,1.0), viewDir)));
    float layerDepth = 1.0/numLayers;
    float currentLayerDepth = 0.0;

    // p => offset
    vec2 P = (viewDir.xy/viewDir.z) * (heightScale);
    vec2 deltaTexCoords = P / numLayers;

    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;

    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main()
{
    vec3 fragToView = fsin.tangentViewPos - fsin.tangentFragPos;
    vec2 texCoords = ParallaxMapping(fsin.texCoords, fragToView);
    if (texCoords.x > 1.0 || texCoords.x < 0.0 || texCoords.y > 1.0 || texCoords.y < 0.0)
    {
        discard;
    }

    //vec3 diffuseColor = texture(diffuseMap, texCoords).rgb;
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize((normal * 2.0) - 1.0); // Map to [-1, 1]

    vec3 color = texture(diffuseMap, texCoords).rgb;
    vec3 fragToLight = normalize(fsin.tangentLightPos - fsin.tangentFragPos);
    float diffuse = max(0.0, dot(normal, fragToLight));
    vec3 diffuseColor = diffuse * color;

    vec3 reflectLight = normalize(reflect(-fragToLight, normal));
    vec3 halfway = normalize(fragToLight + fragToView);
    float specular = pow(max(0.0,dot(halfway, reflectLight)), 32.0);
    vec3 specularColor = vec3(0.2) * specular;

    float ambient = 0.1;
    vec3 ambientColor = ambient * color;

    FragColor = vec4(ambientColor + diffuseColor + specularColor, 1.0);
}
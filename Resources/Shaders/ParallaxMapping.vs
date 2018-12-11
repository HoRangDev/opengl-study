#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT
{
    vec3 fragPosition;
    vec2 texCoords;
    vec3 tangentLightPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
}vsout;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vec3 T = normalize(mat3(model) * aTangent);
    vec3 B = normalize(mat3(model) * aBitangent);
    vec3 N = normalize(mat3(model) * aNormal);
    mat3 TBN = mat3(T,B,N);

    vsout.fragPosition = vec3(model * vec4(aPosition, 1.0));
    vsout.texCoords = aTexCoords;
    vsout.tangentLightPos = TBN * lightPos;
    vsout.tangentViewPos = TBN * viewPos;
    vsout.tangentFragPos = TBN * vsout.fragPosition;

    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}
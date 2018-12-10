#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;

out VSOut
{
    vec3 position;
    vec2 texCoord;
    mat3 TBN;
}vsout;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 B = normalize(vec3(model * vec4(aBiTangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));

    mat3 TBN = mat3(T,B,N);

    vsout.position = vec3(model * vec4(aPosition,1.0));
    vsout.texCoord = aTexCoord;
    vsout.TBN = TBN;
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}
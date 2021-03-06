#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 aInstWorldMat;

layout (std140) uniform matrices
{
	uniform mat4 view;
	uniform mat4 projection;
};

out VSOut
{
	vec3 Normal;
	vec2 TexCoord;
	vec3 FragPos;
} vsout;

void main()
{
	vsout.FragPos = vec3( aInstWorldMat * vec4(aPos, 1.0) );
	vsout.Normal = mat3( transpose( inverse(aInstWorldMat) ) )* aNormal;
	vsout.TexCoord = aTexCoord;
	gl_Position = projection * view * aInstWorldMat * vec4(aPos, 1.0);
}
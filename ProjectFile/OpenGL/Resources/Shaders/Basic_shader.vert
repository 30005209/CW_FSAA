#version 330

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 texCoord;

uniform mat4 model;
uniform mat4 viewProjection;

out vec2 TexCoord;
out vec3 Normal;

void main()
{
	TexCoord = texCoord;
	Normal = vertexNormal;
	gl_Position = viewProjection * model * vec4(vertexPos, 1.0f);
}
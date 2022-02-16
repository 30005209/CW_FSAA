#version 330 core

in vec2 TexCoord;
in vec3 Normal;

// texture sampler
uniform sampler2D texture_diffuse1;

out vec4 FragColour;

void main()
{
	vec4 TextureColour = texture(texture_diffuse1, TexCoord);

	FragColour = TextureColour;
}
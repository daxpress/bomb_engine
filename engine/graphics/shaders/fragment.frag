#version 450

layout (location = 0) out vec4 finalColor;

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 texCoord;

layout (binding = 1) uniform sampler2D texSampler;

void main()
{
	finalColor = texture(texSampler, texCoord) * (fragColor, 1.0);
}
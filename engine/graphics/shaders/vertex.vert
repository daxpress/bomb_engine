#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (binding = 0) uniform UniformBufferObject
{
	vec4 model;
	vec4 view;
	vec4 projection;
} mvp;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoord;

void main()
{
	gl_Position = mvp.projection * mvp.view * mvp.model * vec4(inPosition, 1);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}
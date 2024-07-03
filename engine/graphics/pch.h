// GRAPHICS PCH LIST

#pragma once
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // this tells glm to use (0.0, 1.0) range for depth (Vulkan standard)
#include <glm/glm.hpp>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>

// resources
#include <tiny_obj_loader.h>

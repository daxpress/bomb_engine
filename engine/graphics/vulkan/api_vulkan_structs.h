#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace BE_NAMESPACE
{
struct VkQueueFamilyIndices
{
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;
    std::optional<uint32_t> transfer;
    std::optional<uint32_t> compute;

    inline auto is_complete() -> bool
    {
        return graphics.has_value() && present.has_value() && transfer.has_value() &&
               compute.has_value();
    }
};

struct VkSwapchainDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;

    VkSwapchainDetails(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);
};

struct VkSwapchainInfo
{
    vk::SwapchainKHR swapchain;
    std::vector<vk::Image> images;
    std::vector<vk::ImageView> image_views;
    vk::Format format;
    vk::Extent2D extent;
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};
}  // namespace BE_NAMESPACE
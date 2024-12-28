#pragma once
#include "api_vulkan_structs.h"

namespace BE_NAMESPACE
{
class VulkanQueue
{
public:
    static auto get_families(const vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface)
        -> VkQueueFamilyIndices;

    VulkanQueue(
        const std::shared_ptr<vk::raii::Device>& device, const uint32_t queue_family_index, const uint32_t queue_index = 0
    );

    auto get() const -> std::shared_ptr<vk::raii::Queue> { return m_queue; }

private:
    inline static VkQueueFamilyIndices s_indices;

    std::shared_ptr<vk::raii::Device> m_device = nullptr;
    std::shared_ptr<vk::raii::Queue> m_queue = nullptr;
    uint32_t m_queue_family_index = 0;
};
}  // namespace BE_NAMESPACE

#include "vulkan_queue.h"

#include "api_vulkan_internal.h"

namespace BE_NAMESPACE
{

auto VulkanQueue::get_families(
    const vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface
) -> VkQueueFamilyIndices
{
    // get cached indices (they won't change during gameplay)
    if (s_indices.is_complete())
    {
        return s_indices;
    }

    const auto queue_families = device.getQueueFamilyProperties();

    for (uint32_t family_index = 0; auto& queue_family : queue_families)
    {
        if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            s_indices.graphics = family_index;
        }

        if (device.getSurfaceSupportKHR(family_index, surface))
        {
            s_indices.present = family_index;
        }

        if (!(queue_family.queueFlags & vk::QueueFlagBits::eGraphics) &&
            queue_family.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            s_indices.transfer = family_index;
        }

        if (!(queue_family.queueFlags & vk::QueueFlagBits::eGraphics) &&
            queue_family.queueFlags & vk::QueueFlagBits::eCompute)
        {
            s_indices.compute = family_index;
        }

        if (s_indices.is_complete())
        {
            break;
        }

        family_index++;
    }

    return s_indices;
}

VulkanQueue::VulkanQueue(
    const std::shared_ptr<vk::raii::Device>& device,
    const uint32_t queue_family_index,
    const uint32_t queue_index
)
    : m_device(device), m_queue_family_index(queue_family_index)
{
    if (auto expected_queue = m_device->getQueue(queue_family_index, queue_index);
        expected_queue.has_value())
    {
        m_queue = std::make_shared<vk::raii::Queue>(std::move(expected_queue.value()));
    }
    else
    {
        Log(VulkanAPICategory,
            LogSeverity::Fatal,
            "Failed to get device queue\nqueue_family: {}\nqueue_index: {}!",
            queue_family_index,
            queue_index);
    }
}
}  // namespace BE_NAMESPACE
#include "vulkan_statics.h"

#include "api_vulkan_internal.h"

namespace BE_NAMESPACE
{

auto VulkanStatics::find_memory_type(
    const vk::PhysicalDevice& physical_device,
    const uint32_t type_bits,
    const vk::MemoryPropertyFlags props
) -> uint32_t
{
    const auto pd_props = physical_device.getMemoryProperties();

    for (uint32_t i = 0; i < pd_props.memoryTypeCount; i++)
    {
        if (type_bits & (1 << i) && (pd_props.memoryTypes[i].propertyFlags & props) == props)
        {
            return i;
        }
    }

    Log(VulkanAPICategory, LogSeverity::Fatal, "failed to find suitable memory type!");
    // find a solution to avoid crashing
    std::unreachable();
}
auto VulkanStatics::create_command_buffers(
    const vk::Device& device,
    const vk::CommandPool& pool,
    const vk::CommandBufferLevel level,
    const uint32_t count
) -> std::vector<vk::CommandBuffer>
{
    return device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(pool, level, count));
}
auto VulkanStatics::create_command_buffer(
    const vk::Device& device, const vk::CommandPool& pool, const vk::CommandBufferLevel level
) -> vk::CommandBuffer
{
    return create_command_buffers(device, pool, level, 1)[0];
}
auto VulkanStatics::begin_one_time_commands(const vk::Device& device, const vk::CommandPool& pool)
    -> vk::CommandBuffer
{
    const auto buffer = device.allocateCommandBuffers(
        vk::CommandBufferAllocateInfo(pool, vk::CommandBufferLevel::ePrimary, 1)
    )[0];  // get the first and only element
    buffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    return buffer;
}
void VulkanStatics::end_one_time_commands(
    const vk::Device& device,
    const vk::CommandBuffer& buffer,
    const vk::Queue& queue,
    const vk::CommandPool& pool
)
{
    buffer.end();
    queue.submit(vk::SubmitInfo(nullptr, nullptr, buffer, nullptr));
    queue.waitIdle();

    device.freeCommandBuffers(pool, buffer);
}
}  // namespace BE_NAMESPACE
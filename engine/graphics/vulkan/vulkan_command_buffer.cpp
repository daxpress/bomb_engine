#include "vulkan_command_buffer.h"

#include <queue>

namespace BE_NAMESPACE
{
VulkanCommandBuffer::VulkanCommandBuffer(
    const std::shared_ptr<vk::Device> device, const vk::CommandPool& pool, vk::CommandBuffer buffer
)
    : m_command_buffer(buffer), m_command_pool(pool), m_device(device)
{
}

void VulkanCommandBuffer::begin_record(const vk::CommandBufferUsageFlags usage) const
{
    m_command_buffer.begin(usage);
}
void VulkanCommandBuffer::submit(const vk::Queue& queue) const
{
    m_command_buffer.end();
    queue.submit(vk::SubmitInfo(nullptr, nullptr, m_command_buffer, nullptr));
    queue.waitIdle();
}
// VulkanCommandBuffer::~VulkanCommandBuffer()
// {
//     m_device->freeCommandBuffers(m_command_pool, 1, &m_command_buffer);
// }
}  // namespace BE_NAMESPACE

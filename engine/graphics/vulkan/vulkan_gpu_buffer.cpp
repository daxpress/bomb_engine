#include "vulkan_gpu_buffer.h"

#include "api_vulkan_internal.h"
#include "vulkan_statics.h"

namespace BE_NAMESPACE
{

auto VulkanGpuBufferFactory::create(
    const uint32_t size,
    const vk::BufferUsageFlags usage,
    const vk::SharingMode sharing_mode,
    const vk::MemoryPropertyFlags properties
) -> std::shared_ptr<VulkanGpuBuffer>
{
    auto buffer = m_device->createBuffer(
        vk::BufferCreateInfo(vk::BufferCreateFlags(), size, usage, sharing_mode, m_families)
    );

    const auto mem_req = m_device->getBufferMemoryRequirements(buffer);

    auto buffer_memory = m_device->allocateMemory(
        vk::MemoryAllocateInfo(
            mem_req.size,
            VulkanStatics::find_memory_type(*m_physical_device, mem_req.memoryTypeBits, properties)
        )
    );
    m_device->bindBufferMemory(buffer, buffer_memory, 0);

    return std::make_shared<VulkanGpuBuffer>(
        buffer,
        buffer_memory,
        m_command_pool,
        m_device,
        usage,
        sharing_mode,
        properties,
        size
    );
}

auto VulkanGpuBuffer::copy_to(const VulkanGpuBuffer& other, const vk::Queue& queue) const -> bool
{
    if (other.m_size < m_size)
    {
        return false;
    }

    auto command_buffer = VulkanStatics::create_command_buffer(
        *m_device, *m_command_pool, vk::CommandBufferLevel::ePrimary
    );

    command_buffer.begin(
        vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
    );
    const vk::BufferCopy region(0, 0, m_size);
    command_buffer.copyBuffer(m_buffer, other.m_buffer, region);
    command_buffer.end();

    vk::SubmitInfo submit_info{};
    submit_info.setCommandBuffers(command_buffer);
    queue.submit(submit_info);
    queue.waitIdle();

    m_device->freeCommandBuffers(*m_command_pool, command_buffer);
    return true;
}
VulkanGpuBuffer::~VulkanGpuBuffer()
{
    m_device->freeMemory(m_memory);
    m_device->destroyBuffer(m_buffer);
}
}  // namespace BE_NAMESPACE
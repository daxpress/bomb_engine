#include "vulkan_buffer.h"

#include "vulkan_statics.h"

namespace BE_NAMESPACE
{

VulkanBufferFactory::VulkanBufferFactory(
    const std::vector<uint32_t>& families,
    std::shared_ptr<vk::PhysicalDevice> physical_device,
    std::shared_ptr<vk::Device> device,
    std::shared_ptr<vk::CommandPool> command_pool
)
    : m_families(families),
      m_device(device),
      m_physical_device(physical_device),
      m_command_pool(command_pool)
{
}

auto VulkanBufferFactory::create(
    const uint32_t size,
    const vk::BufferUsageFlags usage,
    const vk::SharingMode sharing_mode,
    const vk::MemoryPropertyFlags properties
) -> std::shared_ptr<VulkanBuffer>
{
    auto buffer = m_device->createBuffer(
        vk::BufferCreateInfo(vk::BufferCreateFlags(), size, usage, sharing_mode, m_families)
    );

    const auto mem_req = m_device->getBufferMemoryRequirements(buffer);

    auto buffer_memory = m_device->allocateMemory(
        vk::MemoryAllocateInfo(
            mem_req.size,
            vulkan_statics::find_memory_type(*m_physical_device, mem_req.memoryTypeBits, properties)
        )
    );
    m_device->bindBufferMemory(buffer, buffer_memory, 0);

    auto new_buffer = VulkanBuffer(
        buffer, buffer_memory, m_command_pool, m_device, usage, sharing_mode, properties, size
    );
    return std::make_shared<VulkanBuffer>(std::move(new_buffer));
}

VulkanBuffer::VulkanBuffer(
    vk::Buffer buffer,
    vk::DeviceMemory memory,
    std::shared_ptr<vk::CommandPool> command_pool,
    std::shared_ptr<vk::Device> device,
    const vk::BufferUsageFlags usage,
    const vk::SharingMode sharing_mode,
    const vk::MemoryPropertyFlags properties,
    const uint32_t size
)
    : m_size(size),
      m_usage(usage),
      m_sharing_mode(sharing_mode),
      m_properties(properties),
      m_buffer(buffer),
      m_memory(memory),
      m_device(device),
      m_command_pool(command_pool)
{
}

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
    : m_size(other.m_size),
      m_usage(other.m_usage),
      m_sharing_mode(other.m_sharing_mode),
      m_properties(other.m_properties),
      m_buffer(std::move(other.m_buffer)),
      m_memory(std::move(other.m_memory)),
      m_device(std::move(other.m_device)),
      m_command_pool(std::move(other.m_command_pool))
{
    // invalidate m_device so that it doesn't touch the gpu memory
    other.m_device = nullptr;
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept
{
    if (this != &other)
    {
        m_buffer = std::move(other.m_buffer);
        m_memory = std::move(other.m_memory);
        m_command_pool = std::move(other.m_command_pool);
        m_device = std::move(other.m_device);
        m_usage = other.m_usage;
        m_sharing_mode = other.m_sharing_mode;
        m_properties = other.m_properties;
        m_size = other.m_size;  // Reset 'other' to a valid state
        other.m_buffer = nullptr;
        other.m_memory = nullptr;
        other.m_command_pool.reset();
        other.m_device.reset();
        other.m_usage = {};
        other.m_sharing_mode = {};
        other.m_properties = {};
        other.m_size = 0;
    }
    return *this;
}

auto VulkanBuffer::copy_to(const VulkanBuffer& other, const vk::Queue& queue) const -> bool
{
    if (other.m_size < m_size)
    {
        return false;
    }

    auto command_buffer = vulkan_statics::command_buffer::create_command_buffer(
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
VulkanBuffer::~VulkanBuffer()
{
    if (!m_device)
    {
        return;
    }

    m_device->freeMemory(m_memory);
    m_device->destroyBuffer(m_buffer);
}
}  // namespace BE_NAMESPACE
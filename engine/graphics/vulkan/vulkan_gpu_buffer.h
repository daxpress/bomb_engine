
#pragma once
#include <any>

namespace BE_NAMESPACE
{
class VulkanGpuBuffer;

class VulkanGpuBufferFactory
{
public:
    VulkanGpuBufferFactory(
        const std::vector<uint32_t>& families,
        std::shared_ptr<vk::PhysicalDevice> physical_device,
        std::shared_ptr<vk::Device> device,
        std::shared_ptr<vk::CommandPool> command_pool
    )
        : m_families(families),
          m_device(device),
          m_physical_device(physical_device),
          m_command_pool(command_pool) {};

    auto create(
        const uint32_t size,
        const vk::BufferUsageFlags usage,
        const vk::SharingMode sharing_mode,
        const vk::MemoryPropertyFlags properties
    ) -> std::shared_ptr<VulkanGpuBuffer>;

private:
    const std::vector<uint32_t> m_families;
    std::shared_ptr<vk::Device> m_device;
    std::shared_ptr<vk::PhysicalDevice> m_physical_device;
    std::shared_ptr<vk::CommandPool> m_command_pool;
};

class VulkanGpuBuffer
{
    friend class VulkanGpuBufferFactory;

public:
    VulkanGpuBuffer(
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
          m_command_pool(command_pool) {};

    [[nodiscard]] auto copy_to(const VulkanGpuBuffer& other, const vk::Queue& queue) const -> bool;

    template <typename Type>
    auto set_data(const std::span<Type> data) -> void
    {
        auto size = data.size() * sizeof(Type);
        const auto mapped = m_device->mapMemory(m_memory, 0, size);
        memcpy(mapped, data.data(), size);
        m_device->unmapMemory(m_memory);
    }

    [[nodiscard]] auto buffer() const -> const vk::Buffer& { return m_buffer; }
    [[nodiscard]] auto memory() const -> const vk::DeviceMemory& { return m_memory; }

    ~VulkanGpuBuffer();

private:
    uint32_t m_size = 0;
    // some redundant info, helpful for copying
    const vk::BufferUsageFlags m_usage;
    const vk::SharingMode m_sharing_mode;
    const vk::MemoryPropertyFlags m_properties;
    // actual useful stuff
    vk::Buffer m_buffer;
    vk::DeviceMemory m_memory;
    std::shared_ptr<vk::Device> m_device;
    std::shared_ptr<vk::CommandPool> m_command_pool;
};
}  // namespace BE_NAMESPACE
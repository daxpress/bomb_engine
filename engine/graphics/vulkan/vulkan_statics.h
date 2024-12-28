#pragma once

namespace BE_NAMESPACE
{
class VulkanStatics
{
public:
    static auto find_memory_type(
        const vk::PhysicalDevice& physical_device,
        const uint32_t type_bits,
        const vk::MemoryPropertyFlags props
    ) -> uint32_t;

    static auto create_command_buffers(
        const vk::Device& device,
        const vk::CommandPool& pool,
        const vk::CommandBufferLevel level,
        const uint32_t count
    ) -> std::vector<vk::CommandBuffer>;

    static auto create_command_buffer(
        const vk::Device& device, const vk::CommandPool& pool, const vk::CommandBufferLevel level
    ) -> vk::CommandBuffer;

    static auto begin_one_time_commands(const vk::Device& device, const vk::CommandPool& pool)
        -> vk::CommandBuffer;

    static void end_one_time_commands(
        const vk::Device& device,
        const vk::CommandBuffer& buffer,
        const vk::Queue& queue,
        const vk::CommandPool& pool
    );
};
}  // namespace BE_NAMESPACE

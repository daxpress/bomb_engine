#pragma once

namespace BE_NAMESPACE
{
struct VkQueueFamilyIndices;

namespace vulkan_statics
{
auto get_queue_families(const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface)
    -> VkQueueFamilyIndices;

auto find_memory_type(
    const vk::PhysicalDevice& physical_device,
    const uint32_t type_bits,
    const vk::MemoryPropertyFlags props
) -> uint32_t;

namespace command_buffer
{
auto create_command_buffers(
    const vk::Device& device,
    const vk::CommandPool& pool,
    const vk::CommandBufferLevel level,
    const uint32_t count
) -> std::vector<vk::CommandBuffer>;

auto create_command_buffer(
    const vk::Device& device, const vk::CommandPool& pool, const vk::CommandBufferLevel level
) -> vk::CommandBuffer;

auto begin_one_time_commands(const vk::Device& device, const vk::CommandPool& pool)
    -> vk::CommandBuffer;

void end_one_time_commands(
    const vk::Device& device,
    const vk::CommandBuffer& buffer,
    const vk::Queue& queue,
    const vk::CommandPool& pool
);
}  // namespace command_buffer

auto create_command_pool(
    const vk::Device& device, const vk::CommandPoolCreateFlags flags, const uint32_t queue_family
) -> vk::CommandPool;

namespace image
{
auto has_stencil_component(vk::Format format) -> bool;

auto create_image_sampler(
    const vk::PhysicalDevice& physical_device,
    const vk::Device& device,
    const vk::Filter filter,
    const vk::SamplerAddressMode address_mode
) -> vk::Sampler;
}  // namespace image

namespace mesh
{
auto get_binding_description() -> vk::VertexInputBindingDescription;
auto get_attribute_descriptions() -> std::vector<vk::VertexInputAttributeDescription>;

}  // namespace mesh
};  // namespace vulkan_statics
}  // namespace BE_NAMESPACE

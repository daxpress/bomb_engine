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
auto VulkanStatics::create_command_pool(
    const vk::Device& device, const vk::CommandPoolCreateFlags flags, const uint32_t queue_family
) -> vk::CommandPool
{
    return device.createCommandPool(vk::CommandPoolCreateInfo(flags, queue_family));
}
auto VulkanStatics::has_stencil_component(vk::Format format) -> bool
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}
auto VulkanStatics::create_image_sampler(
    const vk::PhysicalDevice& physical_device,
    const vk::Device& device,
    const vk::Filter filter,
    const vk::SamplerAddressMode address_mode
) -> vk::Sampler
{
    vk::SamplerCreateInfo sampler_info{};
    sampler_info.magFilter = filter;
    sampler_info.minFilter = filter;

    sampler_info.addressModeU = address_mode;
    sampler_info.addressModeV = address_mode;
    sampler_info.addressModeW = address_mode;

    sampler_info.anisotropyEnable = true;

    auto properties = physical_device.getProperties();

    sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = vk::BorderColor::eIntOpaqueBlack;
    sampler_info.unnormalizedCoordinates = false;
    sampler_info.compareEnable = true;
    sampler_info.compareOp = vk::CompareOp::eAlways;

    sampler_info.mipmapMode = vk::SamplerMipmapMode::eLinear;
    sampler_info.mipLodBias = 1.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = vk::LodClampNone;

    return device.createSampler(sampler_info);
}

}  // namespace BE_NAMESPACE
#include "vulkan_image.h"

#include "api_vulkan_internal.h"
#include "vulkan_statics.h"

namespace BE_NAMESPACE
{
VulkanImageFactory::VulkanImageFactory(
    VkQueueFamilyIndices families,
    std::shared_ptr<vk::PhysicalDevice> physical_device,
    std::shared_ptr<vk::Device> device,
    std::shared_ptr<vk::CommandPool> command_pool,
    std::shared_ptr<vk::Queue> queue
)
    : m_families(families),
      m_physical_device(physical_device),
      m_device(device),
      m_command_pool(command_pool),
      m_queue(queue)
{
}
auto VulkanImageFactory::create(
    const VulkanImageInfo& info, const vk::MemoryPropertyFlags properties
) const -> std::shared_ptr<VulkanImage>
{
    auto sharing_mode = vk::SharingMode::eExclusive;
    std::vector<uint32_t> family_indices{};

    if (m_families.graphics.value() != m_families.transfer.value())
    {
        sharing_mode = vk::SharingMode::eConcurrent;
        family_indices = {m_families.graphics.value(), m_families.transfer.value()};
    }

    const vk::ImageCreateInfo create_info(
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        info.format,
        vk::Extent3D{info.width, info.height, 1},
        info.mips,
        1,
        info.num_samples,
        info.tiling,
        info.usage,
        sharing_mode,
        family_indices
    );
    auto image = m_device->createImage(create_info);

    const auto mem_req = m_device->getImageMemoryRequirements(image);
    const vk::MemoryAllocateInfo alloc_info(
        mem_req.size,
        vulkan_statics::find_memory_type(*m_physical_device, mem_req.memoryTypeBits, properties)
    );
    auto memory = m_device->allocateMemory(alloc_info);

    m_device->bindImageMemory(image, memory, 0);

    auto new_image = VulkanImage(
        std::move(image),
        std::move(memory),
        std::move(info),
        m_physical_device,
        m_device,
        m_command_pool,
        m_queue
    );

    return std::make_shared<VulkanImage>(std::move(new_image));
}

VulkanImage::VulkanImage(
    vk::Image image,
    vk::DeviceMemory memory,
    const VulkanImageInfo& info,
    std::shared_ptr<vk::PhysicalDevice> physical_device,
    std::shared_ptr<vk::Device> device,
    std::shared_ptr<vk::CommandPool> command_pool,
    std::shared_ptr<vk::Queue> queue
)
    : m_image(image),
      m_memory(memory),
      m_info(info),
      m_physical_device(physical_device),
      m_device(device),
      m_command_pool(command_pool),
      m_queue(queue)
{
    make_image_view();
}

VulkanImage::VulkanImage(VulkanImage&& other) noexcept
    : m_image(std::move(other.m_image)),
      m_memory(std::move(other.m_memory)),
      m_image_view(std::move(other.m_image_view)),
      m_info(std::move(other.m_info))
{
    m_physical_device = std::move(other.m_physical_device);
    m_device = std::move(other.m_device);
    m_command_pool = std::move(other.m_command_pool);
    m_queue = std::move(other.m_queue);

    other.m_physical_device.reset();
    other.m_device.reset();
    other.m_command_pool.reset();
    other.m_queue.reset();
}

VulkanImage& VulkanImage::operator=(VulkanImage&& other) noexcept
{
    m_image = std::move(other.m_image);
    m_memory = std::move(other.m_memory);
    m_image_view = std::move(other.m_image_view);
    m_info = std::move(other.m_info);
    m_physical_device = std::move(other.m_physical_device);
    m_device = std::move(other.m_device);
    m_command_pool = std::move(other.m_command_pool);
    m_queue = std::move(other.m_queue);

    other.m_physical_device.reset();
    other.m_device.reset();
    other.m_command_pool.reset();
    other.m_queue.reset();

    return *this;
}

VulkanImage::~VulkanImage()
{
    if (!m_device)
    {
        // allows to create them
        return;
    }

    m_device->destroyImageView(m_image_view);
    m_device->destroyImage(m_image);
    m_device->freeMemory(m_memory);
}

void VulkanImage::generate_mipmaps(const uint32_t mips) const

{
    const auto properties = m_physical_device->getFormatProperties(m_info.format);

    if (!(properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
    {
        Log(VulkanAPICategory,
            LogSeverity::Fatal,
            "texture image format does not support linear binding!");
    }
    const auto command_buffer = vulkan_statics::command_buffer::begin_one_time_commands(*m_device, *m_command_pool);

    vk::ImageMemoryBarrier barrier{};

    barrier.image = m_image;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.subresourceRange =
        vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

    int32_t mip_width = m_info.width;
    int32_t mip_height = m_info.height;

    for (uint32_t i = 1; i < mips; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlags(),
            nullptr,
            nullptr,
            barrier
        );

        vk::ImageBlit blit(
            vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i - 1, 0, 1),
            {vk::Offset3D(0, 0, 0), vk::Offset3D(mip_width, mip_height, 1)},
            vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i, 0, 1),
            {vk::Offset3D(0, 0, 0),
             vk::Offset3D(mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1)
            }
        );

        command_buffer.blitImage(
            m_image,
            vk::ImageLayout::eTransferSrcOptimal,
            m_image,
            vk::ImageLayout::eTransferDstOptimal,
            blit,
            vk::Filter::eLinear
        );

        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::DependencyFlags(),
            nullptr,
            nullptr,
            barrier
        );

        if (mip_width > 1) mip_width /= 2;
        if (mip_height > 1) mip_height /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mips - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    command_buffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eFragmentShader,
        vk::DependencyFlags(),
        nullptr,
        nullptr,
        barrier
    );
    vulkan_statics::command_buffer::end_one_time_commands(*m_device, command_buffer, *m_queue, *m_command_pool);
}

void VulkanImage::make_image_view()
{
    m_image_view = m_device->createImageView(
        vk::ImageViewCreateInfo(
            vk::ImageViewCreateFlags(),
            m_image,
            vk::ImageViewType::e2D,
            m_info.format,
            {},
            vk::ImageSubresourceRange(m_info.aspect, 0, m_info.mips, 0, 1)
        )
    );
}

auto VulkanImage::transition_layout(const vk::ImageLayout new_layout) -> bool
{
    const auto buffer = vulkan_statics::command_buffer::begin_one_time_commands(*m_device, *m_command_pool);

    vk::ImageMemoryBarrier barrier{};

    barrier.oldLayout = m_info.layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image = m_image;
    if (new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        if (vulkan_statics::image::has_stencil_component(m_info.format))
        {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = m_info.mips;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags src_stage;
    vk::PipelineStageFlags dst_stage;

    if (m_info.layout == vk::ImageLayout::eUndefined &&
        new_layout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        dst_stage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (m_info.layout == vk::ImageLayout::eUndefined &&
             new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        src_stage = vk::PipelineStageFlagBits::eTransfer;
        dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (m_info.layout == vk::ImageLayout::eUndefined &&
             new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead |
                                vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        dst_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    }
    else
    {
        Log(VulkanAPICategory, LogSeverity::Fatal, "Unsupported image layout transition!");
        return false;
    }

    buffer.pipelineBarrier(src_stage, dst_stage, vk::DependencyFlags(), nullptr, nullptr, barrier);

    vulkan_statics::command_buffer::end_one_time_commands(*m_device, buffer, *m_queue, *m_command_pool);

    m_info.layout = new_layout;

    return true;
}

void VulkanImage::copy_from_buffer(const VulkanBuffer& buffer)
{
    const auto cmd_buffer = vulkan_statics::command_buffer::begin_one_time_commands(*m_device, *m_command_pool);

    const vk::BufferImageCopy region(
        0,
        0,
        0,
        vk::ImageSubresourceLayers(m_info.aspect, 0, 0, 1),
        {0, 0, 0},
        {m_info.width, m_info.height, 1}
    );

    cmd_buffer.copyBufferToImage(
        buffer.buffer(), m_image, vk::ImageLayout::eTransferDstOptimal, region
    );
    vulkan_statics::command_buffer::end_one_time_commands(*m_device, cmd_buffer, *m_queue, *m_command_pool);

    if (m_info.mips > 1)
    {
        generate_mipmaps(m_info.mips);
    }
}
}  // namespace BE_NAMESPACE
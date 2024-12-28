#pragma once
#include "api_vulkan_structs.h"
#include "vulkan_gpu_buffer.h"

namespace BE_NAMESPACE
{

struct VulkanImageInfo
{
    uint32_t width;
    uint32_t height;
    uint32_t mips;
    vk::SampleCountFlagBits num_samples;
    vk::Format format;
    vk::ImageTiling tiling;
    vk::ImageUsageFlags usage;
    vk::ImageLayout layout;
    vk::ImageAspectFlags aspect;
};

class VulkanImage;

class VulkanImageFactory
{
public:
    VulkanImageFactory(
        VkQueueFamilyIndices families,
        std::shared_ptr<vk::PhysicalDevice> physical_device,
        std::shared_ptr<vk::Device> device,
        std::shared_ptr<vk::CommandPool> command_pool,
        std::shared_ptr<vk::Queue> queue
    );

    auto create(const VulkanImageInfo& info, const vk::MemoryPropertyFlags properties) const
        -> std::shared_ptr<VulkanImage>;

private:
    const VkQueueFamilyIndices m_families;
    std::shared_ptr<vk::Device> m_device;
    std::shared_ptr<vk::PhysicalDevice> m_physical_device;
    std::shared_ptr<vk::CommandPool> m_command_pool;
    std::shared_ptr<vk::Queue> m_queue;
};

class VulkanImage
{
    friend class VulkanImageFactory;

    VulkanImage(
        vk::Image image,
        vk::DeviceMemory memory,
        const VulkanImageInfo& info,
        std::shared_ptr<vk::PhysicalDevice> physical_device,
        std::shared_ptr<vk::Device> device,
        std::shared_ptr<vk::CommandPool> command_pool,
        std::shared_ptr<vk::Queue> queue
    );

    void generate_mipmaps(uint32_t mips) const;
    void make_image_view();

public:
    VulkanImage(VulkanImage&& other) noexcept;
    auto operator=(VulkanImage&& other) noexcept -> VulkanImage&;
    ~VulkanImage();

    auto transition_layout(const vk::ImageLayout new_layout) -> bool;
    void copy_from_buffer(const VulkanGpuBuffer& buffer);

    [[nodiscard]] auto info() const -> VulkanImageInfo { return m_info; }
    [[nodiscard]] auto image() const -> vk::Image { return m_image; }
    [[nodiscard]] auto view() const -> vk::ImageView { return m_image_view; }

private:
    vk::Image m_image;
    vk::DeviceMemory m_memory;
    vk::ImageView m_image_view;
    VulkanImageInfo m_info;

    std::shared_ptr<vk::PhysicalDevice> m_physical_device;
    std::shared_ptr<vk::Device> m_device;
    std::shared_ptr<vk::CommandPool> m_command_pool;
    std::shared_ptr<vk::Queue> m_queue;
};
}  // namespace BE_NAMESPACE
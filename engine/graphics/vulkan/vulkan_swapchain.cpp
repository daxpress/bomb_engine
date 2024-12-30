#include "vulkan_swapchain.h"

#include "api_vulkan_structs.h"
#include "vulkan_statics.h"
#include "window.h"

namespace BE_NAMESPACE
{

auto VulkanSwapchain::create(
    std::shared_ptr<vk::PhysicalDevice> physical_device,
    std::shared_ptr<vk::Device> device,
    Window& window,
    vk::SurfaceKHR surface,
    vk::SwapchainKHR old_swapchain
) -> std::shared_ptr<VulkanSwapchain>
{
    auto swapchain_details = VkSwapchainDetails(*physical_device, surface);
    auto [format, color_space] = choose_swapchain_surface_format(swapchain_details.formats);
    auto present_mode = choose_swapchain_present_mode(swapchain_details.present_modes);
    auto extent = choose_swapchain_extent(swapchain_details.capabilities, window);

    uint32_t image_count = swapchain_details.capabilities.minImageCount + 1;
    if (swapchain_details.capabilities.maxImageCount > 0 &&
        image_count > swapchain_details.capabilities.maxImageCount)
    {
        image_count = swapchain_details.capabilities.maxImageCount;
    }

    auto indices = vulkan_statics::get_queue_families(*physical_device, surface);
    std::vector<uint32_t> swapchain_sharing_indices{};
    uint32_t sharing_index_count = 0;
    auto sharing_mode = vk::SharingMode::eExclusive;
    if (indices.graphics != indices.present)
    {
        sharing_index_count = 2;
        swapchain_sharing_indices = {indices.graphics.value(), indices.present.value()};
        sharing_mode = vk::SharingMode::eConcurrent;
    }

    vk::SwapchainCreateInfoKHR create_info(
        vk::SwapchainCreateFlagsKHR(),
        surface,
        image_count,
        format,
        color_space,
        extent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        sharing_mode,
        sharing_index_count,
        swapchain_sharing_indices.data(),
        swapchain_details.capabilities.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        present_mode,
        true,
        old_swapchain,
        nullptr
    );

    VulkanSwapchain swapchain_info(window);
    swapchain_info.m_swapchain = device->createSwapchainKHR(create_info);
    swapchain_info.m_extent = extent;
    swapchain_info.m_format = format;
    swapchain_info.m_images = device->getSwapchainImagesKHR(swapchain_info.m_swapchain);

    swapchain_info.m_image_views.reserve(swapchain_info.m_images.size());
    vk::ImageViewCreateInfo image_view_create_info(
        vk::ImageViewCreateFlags(),
        {},
        vk::ImageViewType::e2D,
        format,
        {},
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
    );

    for (const auto& image : swapchain_info.m_images)
    {
        image_view_create_info.image = image;
        swapchain_info.m_image_views.push_back(device->createImageView(image_view_create_info));
    }

    swapchain_info.m_device = std::move(device);
    swapchain_info.m_surface = std::move(surface);
    swapchain_info.m_physical_device = std::move(physical_device);

    return std::make_shared<VulkanSwapchain>(std::move(swapchain_info));
}

VulkanSwapchain::VulkanSwapchain(VulkanSwapchain&& other) noexcept : m_window(other.m_window)
{
    m_physical_device = std::move(other.m_physical_device);
    m_device = std::move(other.m_device);
    m_surface = std::move(other.m_surface);

    m_extent = other.m_extent;
    m_format = other.m_format;
    m_images = std::move(other.m_images);
    m_image_views = std::move(other.m_image_views);
    m_swapchain = std::move(other.m_swapchain);

    other.m_device.reset();
    other.m_physical_device.reset();
    other.m_surface = nullptr;
    other.m_swapchain = nullptr;
}
VulkanSwapchain& VulkanSwapchain::operator=(VulkanSwapchain&& other) noexcept
{
    if (this != &other)
    {
        m_physical_device = std::move(other.m_physical_device);
        m_device = std::move(other.m_device);
        m_surface = std::move(other.m_surface);
        m_window = other.m_window;

        m_extent = other.m_extent;
        m_format = other.m_format;
        m_images = std::move(other.m_images);
        m_image_views = std::move(other.m_image_views);
        m_swapchain = std::move(other.m_swapchain);

        other.m_device.reset();
        other.m_physical_device.reset();
        other.m_surface = nullptr;
        other.m_swapchain = nullptr;
    }
    return *this;
}

VulkanSwapchain::~VulkanSwapchain()
{
    if (!m_device)
    {
        // in case of move
        return;
    }
    free_views();
    free_swapchain();
}

auto VulkanSwapchain::recreate() -> void
{
    free_views();

    const auto new_swapchain =
        create(m_physical_device, m_device, m_window, m_surface, m_swapchain);

    // we still have to destroy the old swapchain
    free_swapchain();
    m_swapchain = std::move(new_swapchain->m_swapchain);
    m_extent = new_swapchain->m_extent;
    m_format = new_swapchain->m_format;
    m_images = std::move(new_swapchain->m_images);
    m_image_views = std::move(new_swapchain->m_image_views);

    // reset to avoid freeing swapchain stuff
    new_swapchain->m_device.reset();
}

auto VulkanSwapchain::choose_swapchain_surface_format(const std::span<vk::SurfaceFormatKHR> formats)
    -> vk::SurfaceFormatKHR
{
    for (const auto& format : formats)
    {
        if (format.format == vk::Format::eB8G8R8A8Srgb &&
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return format;
        }
    }
    return formats[0];
}
auto VulkanSwapchain::choose_swapchain_present_mode(
    const std::span<vk::PresentModeKHR> present_modes
) -> vk::PresentModeKHR
{
    for (const auto& present_mode : present_modes)
    {
        if (present_mode == vk::PresentModeKHR::eMailbox)  // to render frames as fast as
                                                           // possible
        {
            return present_mode;
        }
    }
    // guaranteed by spec
    return vk::PresentModeKHR::eFifo;
}
auto VulkanSwapchain::choose_swapchain_extent(
    const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window
) -> vk::Extent2D
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window.get_raw_window(), &width, &height);

    const vk::Extent2D extent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    return std::clamp(extent, capabilities.minImageExtent, capabilities.maxImageExtent);
}

void VulkanSwapchain::free_swapchain() const { m_device->destroySwapchainKHR(m_swapchain); }
void VulkanSwapchain::free_views() const
{
    for (const auto& image_view : m_image_views)
    {
        m_device->destroyImageView(image_view);
    }
}
}  // namespace BE_NAMESPACE
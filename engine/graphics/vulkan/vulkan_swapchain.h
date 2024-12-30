#pragma once
#include "vulkan_image.h"

namespace bomb_engine
{
class Window;
}
namespace BE_NAMESPACE
{
class VulkanSwapchain
{
    explicit VulkanSwapchain(Window& window) : m_window(window) {}

public:
    static auto create(
        std::shared_ptr<vk::PhysicalDevice> physical_device,
        std::shared_ptr<vk::Device> device,
        Window& window,
        vk::SurfaceKHR surface,
        vk::SwapchainKHR old_swapchain = nullptr  // mainly for swapchain recreation
    ) -> std::shared_ptr<VulkanSwapchain>;

    VulkanSwapchain(VulkanSwapchain&& other) noexcept;
    VulkanSwapchain& operator=(VulkanSwapchain&& other) noexcept;
    ~VulkanSwapchain();

    auto recreate() -> void;

private:
    static auto choose_swapchain_surface_format(std::span<vk::SurfaceFormatKHR> formats)
        -> vk::SurfaceFormatKHR;

    static auto choose_swapchain_present_mode(const std::span<vk::PresentModeKHR> present_modes)
        -> vk::PresentModeKHR;

    static auto choose_swapchain_extent(
        const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window
    ) -> vk::Extent2D;

    void free_swapchain() const;
    void free_views() const;

public:
    vk::SwapchainKHR m_swapchain;
    std::vector<vk::Image> m_images;
    std::vector<vk::ImageView> m_image_views;
    vk::Format m_format = vk::Format::eUndefined;
    vk::Extent2D m_extent;

private:
    std::shared_ptr<vk::PhysicalDevice> m_physical_device;
    std::shared_ptr<vk::Device> m_device;
    vk::SurfaceKHR m_surface;
    Window& m_window;
};
}  // namespace BE_NAMESPACE
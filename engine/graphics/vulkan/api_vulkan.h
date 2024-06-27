#pragma once

#include "api_interface.h"
#include "spirv_shader.h"
#include "vulkan/api_vulkan_structs.h"
#include "window.h"

namespace BE_NAMESPACE
{
class APIVulkan : public IAPI
{
public:
    APIVulkan(Window& window, bool enable_validation_layers);
    ~APIVulkan() override;

    inline auto get_api() -> E_API override { return E_API::API_VULKAN; }

    // testing only, I need a starting point to work from, which in this case is a model drawn to
    // the surface
    void draw_frame() override;

private:
    const std::vector<const char*> m_validation_layers{"VK_LAYER_KHRONOS_validation"};

    const std::vector<const char*> m_required_device_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    const vk::Format DEPTH_FORMAT = vk::Format::eD32SfloatS8Uint;
    const vk::ClearValue CLEAR_VALUE = vk::ClearValue({0.0f, 0.0f, 0.0f, 1.0f});
    const int MAX_FRAMES_IN_FLIGHT = 2;

    bool b_use_validation_layers = false;
    Window& m_window_ref;
    vk::UniqueInstance m_vulkan_instance;
    VkSurfaceKHR m_surface;
    vk::PhysicalDevice m_physical_device;
    vk::Device m_device;
    vk::Queue m_graphics_queue;
    vk::Queue m_present_queue;
    vk::Queue m_transfer_queue;
    vk::Queue m_compute_queue;
    VkSwapchainInfo m_swapchain_info;
    std::vector<vk::Framebuffer> m_frame_buffers;
    uint32_t current_frame = 0;

    // for pipeline
    vk::Image m_color_image;
    vk::ImageView m_color_image_view;
    vk::DeviceMemory m_color_image_memory;
    vk::Image m_depth_image;
    vk::ImageView m_depth_image_view;
    vk::DeviceMemory m_depth_image_memory;

    std::vector<vk::Semaphore> m_swapchain_image_available;
    std::vector<vk::Semaphore> m_render_finished;
    std::vector<vk::Fence> m_in_flight;
    bool frame_resized = false;

    // example pipeline
    vk::Pipeline m_example_pipeline;
    vk::PipelineLayout m_example_layout;
    vk::RenderPass m_example_renderpass;
    vk::DescriptorSetLayout m_example_descriptor_set_layout;
    vk::CommandPool m_example_command_pool;
    std::vector<vk::CommandBuffer> m_example_command_buffers;
    // model related
    vk::Buffer m_camaro_vb;
    vk::DeviceMemory m_camaro_vb_memory;

private:
    void create_instance(const Window& window, bool enable_validation_layers);
    /// <summary>
    /// Use this to setup a debug callback
    /// </summary>
    void setup_debug_messenger(
        vk::UniqueInstance& instance, vk::DebugUtilsMessengerCreateInfoEXT& messenger_info
    );

    void create_surface(const Window& window, VkSurfaceKHR& surface);

    auto select_physical_device() -> vk::PhysicalDevice;
    /// <summary>
    /// Rates a physical device to pick the best one from the available ones.
    /// </summary>
    /// <param name="physical_device:"> the physical device to rate</param>
    /// <returns> the overall score based on the features and properties</returns>
    auto rate_physical_device(vk::PhysicalDevice physical_device) -> uint32_t;
    auto physical_device_is_suitable(vk::PhysicalDevice physical_device) -> bool;
    auto get_queue_families(vk::PhysicalDevice physical_device) -> VkQueueFamilyIndices;
    auto check_extensions_support(vk::PhysicalDevice physical_device) -> bool;

    auto create_logical_device(vk::PhysicalDevice physical_device) -> vk::Device;

    auto create_swapchain(
        vk::PhysicalDevice physical_device,
        vk::SurfaceKHR surface,
        vk::Device device,
        vk::SwapchainKHR old_swapchain = nullptr
    ) -> VkSwapchainInfo;

    void cleanup_swapchain(VkSwapchainInfo& swapchain);

    auto recreate_swapchain_and_framebuffers(
        vk::PhysicalDevice physical_device,
        vk::SurfaceKHR surface,
        vk::Device device,
        vk::SwapchainKHR old_swapchain,
        vk::RenderPass render_pass
    ) -> std::tuple<VkSwapchainInfo, std::vector<vk::Framebuffer>>;

    auto choose_swapchain_surface_format(std::vector<vk::SurfaceFormatKHR> formats
    ) -> vk::SurfaceFormatKHR;

    auto choose_swapchain_present_mode(const std::vector<vk::PresentModeKHR>& present_modes
    ) -> vk::PresentModeKHR;

    auto choose_swapchain_extent(vk::SurfaceCapabilitiesKHR capabilities) -> vk::Extent2D;

    auto create_shader_module(SPIRVShader& shader) -> vk::ShaderModule;

    auto create_image(
        uint32_t width,
        uint32_t height,
        uint32_t mips,
        vk::SampleCountFlagBits num_samples,
        vk::Format format,
        vk::ImageTiling tiling,
        vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags
    ) -> std::pair<vk::Image, vk::DeviceMemory>;

    auto find_memory_type(uint32_t type_bits, vk::MemoryPropertyFlags props) -> uint32_t;
    void transition_image_layout(
        vk::Image image,
        vk::Format format,
        vk::ImageLayout old_layout,
        vk::ImageLayout new_layout,
        uint32_t mips
    );

    // these are required to get ourselves to draw something in the editor in order to have easier
    // refactoring and feature introduction
    auto create_example_pipeline() -> vk::Pipeline;
    auto create_example_render_pass() -> vk::RenderPass;
    auto create_example_pipeline_layout() -> vk::PipelineLayout;

    void create_color_resources(
        const VkSwapchainInfo& swapchain,
        vk::Image& out_image,
        vk::ImageView& out_img_view,
        vk::DeviceMemory& out_memory
    );
    void create_depth_resources(
        const VkSwapchainInfo& swapchain,
        vk::Image& out_image,
        vk::ImageView& out_img_view,
        vk::DeviceMemory& out_memory
    );
    auto create_frame_buffers(VkSwapchainInfo swapchain, vk::RenderPass render_pass)
        -> std::vector<vk::Framebuffer>;

    auto create_command_pool(vk::CommandPoolCreateFlags flags, uint32_t queue_family)
        -> vk::CommandPool;

    auto create_command_buffers(vk::CommandPool pool, vk::CommandBufferLevel level, uint32_t count)
        -> std::vector<vk::CommandBuffer>;

    auto create_command_buffer(vk::CommandPool pool, vk::CommandBufferLevel level)
        -> vk::CommandBuffer;

    void record_example_command_buffer(vk::CommandBuffer buffer, uint32_t image_index);

    void create_sync_objects();

    std::tuple<vk::Buffer, vk::DeviceMemory> create_buffer(
        uint32_t size,
        vk::BufferUsageFlags usage,
        vk::SharingMode sharing_mode,
        vk::MemoryPropertyFlags properties
    );
};
}  // namespace BE_NAMESPACE
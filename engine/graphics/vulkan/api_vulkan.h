#pragma once

#include "api_interface.h"
#include "mesh.h"
#include "spirv_shader.h"
#include "vulkan/api_vulkan_structs.h"
#include "vulkan/vulkan_swapchain.h"
#include "vulkan_buffer.h"
#include "vulkan_image.h"
#include "window.h"

namespace BE_NAMESPACE
{
class APIVulkan : public IAPI
{
public:
    APIVulkan(Window& window, bool enable_validation_layers);
    ~APIVulkan() override;

    auto get_api() -> E_API override { return E_API::API_VULKAN; }
    void framebuffer_resized() { m_frame_resized = true; }

    // testing only, I need a starting point to work from, which in this case is a model drawn to
    // the surface
    void draw_frame() override;

private:
    const std::vector<const char*> m_validation_layers{"VK_LAYER_KHRONOS_validation"};

    const std::vector<const char*> m_required_device_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    const vk::Format DEPTH_FORMAT = vk::Format::eD32SfloatS8Uint;
    const std::array<vk::ClearValue, 2> CLEAR_VALUES = {
        vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f), vk::ClearDepthStencilValue(1.0f, 0.0f)
    };
    constexpr static uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    bool b_use_validation_layers = false;
    Window& m_window_ref;
    vk::UniqueInstance m_vulkan_instance;
    VkSurfaceKHR m_surface;
    std::shared_ptr<vk::PhysicalDevice> m_physical_device;
    std::shared_ptr<vk::Device> m_device;
    std::shared_ptr<vk::Queue> m_graphics_queue;
    std::shared_ptr<vk::Queue> m_present_queue;
    std::shared_ptr<vk::Queue> m_transfer_queue;
    std::shared_ptr<vk::Queue> m_compute_queue;
    std::shared_ptr<VulkanSwapchain> m_swapchain_info;
    std::vector<vk::Framebuffer> m_frame_buffers;
    uint32_t m_current_frame = 0;
    vk::SampleCountFlagBits m_msaa_samples = vk::SampleCountFlagBits::e1;

    // for pipeline
    std::shared_ptr<VulkanImage> m_color_image;
    std::shared_ptr<VulkanImage> m_depth_image;

    std::vector<vk::Semaphore> m_swapchain_image_available;
    std::vector<vk::Semaphore> m_render_finished;
    std::vector<vk::Fence> m_in_flight;
    bool m_frame_resized = false;

    // example pipeline
    vk::Pipeline m_example_pipeline;
    vk::PipelineLayout m_example_layout;
    vk::RenderPass m_example_renderpass;
    vk::DescriptorSetLayout m_example_descriptor_set_layout;
    std::shared_ptr<vk::CommandPool> m_example_command_pool;
    std::vector<vk::CommandBuffer> m_example_command_buffers;
    vk::DescriptorPool m_example_desc_pool;
    std::vector<vk::DescriptorSet> m_example_desc_sets;
    vk::Sampler m_example_sampler;
    std::shared_ptr<VulkanImage> m_example_image;
    uint32_t m_example_mips;
    // model related
    std::shared_ptr<Mesh> m_model;
    std::shared_ptr<VulkanBuffer> m_model_vb;
    std::shared_ptr<VulkanBuffer> m_model_ib;

    std::vector<std::shared_ptr<VulkanBuffer>> m_uniform_buffers;
    std::vector<std::any> m_uniform_buffers_mapped;

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
    auto check_extensions_support(vk::PhysicalDevice physical_device) -> bool;
    auto get_sample_count() -> vk::SampleCountFlagBits;

    auto create_logical_device(vk::PhysicalDevice physical_device) -> vk::Device;

    auto recreate_swapchain_and_framebuffers(vk::RenderPass render_pass) -> void;

    auto create_shader_module(SPIRVShader& shader) -> vk::ShaderModule;

    // these are required to get ourselves to draw something in the editor in order to have easier
    // refactoring and feature introduction
    auto create_example_pipeline() -> vk::Pipeline;
    auto create_example_render_pass() -> vk::RenderPass;
    auto create_example_pipeline_layout() -> vk::PipelineLayout;
    void create_example_vb();
    void create_example_ib();
    void create_example_uniform_buffers();
    void update_uniform_buffer(uint32_t image_index);
    void populate_example_desc_sets();
    void create_example_texture();
    void draw_example_frame();

    void create_color_resources(const VulkanSwapchain& swapchain);
    void create_depth_resources(const VulkanSwapchain& swapchain);

    auto create_frame_buffers(const VulkanSwapchain& swapchain, vk::RenderPass render_pass)
        -> std::vector<vk::Framebuffer>;

    void record_example_command_buffer(vk::CommandBuffer& buffer, uint32_t image_index);

    void create_sync_objects();

    auto create_descriptor_pool(vk::DescriptorType type, uint32_t size) -> vk::DescriptorPool;
    auto create_descriptor_sets(uint32_t count) -> std::vector<vk::DescriptorSet>;
};
}  // namespace BE_NAMESPACE
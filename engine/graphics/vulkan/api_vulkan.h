#pragma once

#include "api_interface.h"
#include "window.h"
#include "vulkan/api_vulkan_structs.h"
#include "spirv_shader.h"

namespace BE_NAMESPACE
{
	class APIVulkan : public IAPI
	{
	public:
		APIVulkan(Window& window, bool enable_validation_layers);
		virtual ~APIVulkan() override;

		inline virtual E_API get_api() { return E_API::API_VULKAN; }
		
		// testing only, I need a starting point to work from, which in this case is a model drawn to the surface
		virtual void draw_frame() override;


	private:
		const std::vector<const char*> m_validation_layers{
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> m_required_device_extensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

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

		vk::Pipeline m_example_pipeline;
		vk::PipelineLayout m_example_layout;
		vk::RenderPass m_example_renderpass;
		vk::DescriptorSetLayout m_example_descriptor_set_layout;

	private:

		void create_instance(const Window& window, bool enable_validation_layers);
		/// <summary>
		/// Use this to setup a debug callback
		/// </summary>
		void setup_debug_messenger(vk::UniqueInstance& instance, vk::DebugUtilsMessengerCreateInfoEXT& messenger_info);

		void create_surface(const Window& window, VkSurfaceKHR& surface);

		vk::PhysicalDevice select_physical_device();
		/// <summary>
		/// Rates a physical device to pick the best one from the available ones.
		/// </summary>
		/// <param name="physical_device:"> the physical device to rate</param>
		/// <returns> the overall score based on the features and properties</returns>
		uint32_t rate_physical_device(vk::PhysicalDevice physical_device);
		bool physical_device_is_suitable(vk::PhysicalDevice physical_device);
		VkQueueFamilyIndices get_queue_families(vk::PhysicalDevice physical_device);
		bool check_extensions_support(vk::PhysicalDevice physical_device);

		vk::Device create_logical_device(vk::PhysicalDevice physical_device);

		VkSwapchainInfo create_swapchain(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, vk::Device device, vk::SwapchainKHR old_swapchain = nullptr);
		vk::SurfaceFormatKHR choose_swapchain_surface_format(std::vector<vk::SurfaceFormatKHR> formats);
		vk::PresentModeKHR choose_swapchain_present_mode(std::vector<vk::PresentModeKHR> present_modes);
		vk::Extent2D choose_swapchain_extent(vk::SurfaceCapabilitiesKHR capabilities);

		vk::ShaderModule create_shader_module(SPIRVShader& shader);
		// this is required to get ourselves to draw something in the editor in order to have easier refactoring and feature introduction
		vk::Pipeline create_example_pipeline();
		vk::RenderPass create_example_render_pass();
		vk::PipelineLayout create_example_pipeline_layout();
	};
}
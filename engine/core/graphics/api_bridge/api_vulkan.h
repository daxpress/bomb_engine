#pragma once

#include "core/graphics/api_bridge/api_interface.h"
#include "core/graphics/window.h"
#include "core/graphics/api_bridge/api_vulkan_structs.h"

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core::graphics::api
{
	class APIVulkan : public IAPI
	{
	public:
		APIVulkan(const Window& window, bool enable_validation_layers);
		virtual ~APIVulkan() override;

		inline virtual E_API get_api() { return E_API::API_VULKAN; }


	private:
		const std::vector<const char*> m_validation_layers{
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> m_required_device_extensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		bool b_use_validation_layers = false;

		vk::UniqueInstance m_vulkan_instance;
		VkSurfaceKHR m_surface;
		vk::PhysicalDevice m_physical_device;
		vk::Device m_device;
		vk::Queue m_graphics_queue;
		vk::Queue m_present_queue;
		vk::Queue m_transfer_queue;
		vk::Queue m_compute_queue;

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
		QueueFamilyIndices get_queue_families(vk::PhysicalDevice physical_device);
		bool check_extensions_support(vk::PhysicalDevice physical_device);

		vk::Device create_logical_device(vk::PhysicalDevice physical_device);
	};
}
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

		const std::vector<std::string> m_required_device_extensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		vk::UniqueInstance m_vulkan_instance;
		VkSurfaceKHR m_surface;
		vk::PhysicalDevice m_phsycal_device;

	private:

		void create_instance(const Window& window, bool enable_validation_layers);
		/// <summary>
		/// Use this to setup a debug callback
		/// </summary>
		void setup_debug_messenger(vk::UniqueInstance& instance, vk::DebugUtilsMessengerCreateInfoEXT& messenger_info);

		void create_surface(const Window& window, VkSurfaceKHR& surface);

		void select_physical_device();
		bool physical_device_is_suitable(vk::PhysicalDevice physical_device);
		uint32_t rate_physical_device(vk::PhysicalDevice physical_device);
		QueueFamilyIndices get_queue_families(vk::PhysicalDevice physical_device);
		bool check_extensions_support(vk::PhysicalDevice physical_device);
	};
}
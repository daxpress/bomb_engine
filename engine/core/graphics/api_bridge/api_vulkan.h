#pragma once

#include "core/graphics/api_bridge/api_interface.h"
#include "core/graphics/window.h"

#include <vector>
#include <vulkan/vulkan.hpp>

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

		vk::UniqueInstance m_vulkan_instance;
		VkSurfaceKHR m_surface;

	private:

		void create_instance(const Window& window, bool enable_validation_layers);
		/// <summary>
		/// Use this to setup a debug callback
		/// </summary>
		void setup_debug_messenger(vk::UniqueInstance& instance, vk::DebugUtilsMessengerCreateInfoEXT& messenger_info);

		void create_surface(const Window& window, VkSurfaceKHR& surface);
	};
}
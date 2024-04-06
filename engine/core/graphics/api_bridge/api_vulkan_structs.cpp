#include "api_vulkan_structs.h"

namespace core::graphics::api
{
	VkSwapchainDetails::VkSwapchainDetails(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
	{
		capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
		formats = physical_device.getSurfaceFormatsKHR(surface);
		present_modes = physical_device.getSurfacePresentModesKHR(surface);
	}
}
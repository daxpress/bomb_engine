#pragma once

#include <vulkan/vulkan.hpp>

#include <cinttypes>
#include <optional>

namespace core::graphics::api
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphics;
		std::optional<uint32_t> present;
		std::optional<uint32_t> transfer;
		std::optional<uint32_t> compute;

		inline bool is_complete() {
			return graphics.has_value() &&
				present.has_value() &&
				transfer.has_value() &&
				compute.has_value();
		}
	};

	struct SwapchainDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> present_modes;

		SwapchainDetails(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);
	};
}
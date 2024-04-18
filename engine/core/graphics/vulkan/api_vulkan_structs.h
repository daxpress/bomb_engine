#pragma once

#include <vulkan/vulkan.hpp>

#include <cinttypes>
#include <optional>

namespace bomb_engine
{
	struct VkQueueFamilyIndices
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

	struct VkSwapchainDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> present_modes;

		VkSwapchainDetails(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);
	};

	struct VkSwapchainInfo
	{
		vk::SwapchainKHR swapchain;
		std::vector<vk::Image> images;
		std::vector<vk::ImageView> image_views;
		vk::Format format;
		vk::Extent2D extent;
	};
}
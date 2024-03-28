#include "core/graphics/renderer.h"

#include <GLFW/glfw3.h>

#include <vector>
#include <string>
// TODO: remove exceptions and prints, replace with logging system
#include <stdexcept>
#include <iostream>

#pragma region vulkan instance helpers

bool validation_layers_supported(std::vector<const char*> validation_layers)
{
	// same tricks used with the other check above
	uint32_t layer_count;
	if (vk::enumerateInstanceLayerProperties(&layer_count, nullptr) != vk::Result::eSuccess)
	{
		return false;
	}
	std::vector<vk::LayerProperties> available_layers(layer_count);
	if (vk::enumerateInstanceLayerProperties(&layer_count, available_layers.data()) != vk::Result::eSuccess)
	{
		return false;
	}

	for (const auto& layer : validation_layers)
	{
		bool layer_found = false;

		for (const auto& availableLayer : available_layers)
		{
			if (strcmp(layer, availableLayer.layerName) == 0)
			{
				layer_found = true;
			}
		}

		if (!layer_found)
		{
			return false;
		}
	}
	return true;
}

bool extensions_supported(const char** required_extensions, uint32_t count)
{
	uint32_t extensions_count = 0;
	if (vk::enumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr) != vk::Result::eSuccess)
	{
		return false;
	}
	std::vector<vk::ExtensionProperties> extensions(extensions_count);
	// not sure why it doesn't like the hpp version with data in
	if (vk::enumerateInstanceExtensionProperties(nullptr, &extensions_count, extensions.data()) != vk::Result::eSuccess)
	{
		return false;
	}

	for (int i = 0; i < count; i++)
	{
		bool found = false;
		for (const auto& avExtension : extensions)
		{
			if (strcmp(avExtension.extensionName, required_extensions[i]) == 0)
			{
				found = true;
			}
		}
		if (!found)
		{
			return false;
		}
	}
	return true;
}

std::vector<const char*> get_required_instance_extensions(bool enable_validation_layers)
{
	uint32_t extension_count = 0;
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);

	std::vector<const char*> extensions(glfw_extensions, glfw_extensions + extension_count);

	if (enable_validation_layers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

// The debug callback for the validation layers: it has to be either a function or a static method (no context)
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

void fill_debug_utils_messenger_create_info(vk::DebugUtilsMessengerCreateInfoEXT& messenger_info)
{

	messenger_info.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
	messenger_info.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	messenger_info.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral;
	messenger_info.pfnUserCallback = &debug_callback;
	// at the moment we keep it to null ,when we have a bit of framework we might pass in something
	messenger_info.pUserData = nullptr;
}

#pragma endregion vulkan instance helpers

namespace core::graphics
{
	Render::Render(const Window& window, bool enable_validation_layers)
	{
		create_instance(window, enable_validation_layers);
		create_surface(window, m_surface);
	}

	Render::~Render()
	{
		vkDestroySurfaceKHR(*m_vulkan_instance, m_surface, nullptr);
	}

	void Render::create_instance(const Window& window, bool enable_validation_layers)
	{
		vk::ApplicationInfo appInfo(
			window.get_title().c_str(),
			vk::makeVersion(0, 1, 0),
			"Bomb Engine",
			vk::makeVersion(0, 1, 0),
			vk::ApiVersion13,
			nullptr
		);

		vk::InstanceCreateInfo createinfo{};
		createinfo.sType = vk::StructureType::eInstanceCreateInfo;
		createinfo.pApplicationInfo = &appInfo;
		createinfo.pNext = nullptr;

		// get the glfw extensions required for Vulkan
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions;
		glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		if (!extensions_supported(glfw_extensions, glfw_extension_count))
		{
			throw std::runtime_error("required glfw extensions are not supported!");
		}

		// get the final list of all the extensions (conditionally adds in the validation layers)
		auto extensions = get_required_instance_extensions(enable_validation_layers);
		createinfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createinfo.ppEnabledExtensionNames = extensions.data();

		if (enable_validation_layers && !validation_layers_supported(m_validation_layers))
		{
			throw std::runtime_error("requested validation layers are not supported!");
		}

		vk::DebugUtilsMessengerCreateInfoEXT messenger_info{};
		if (enable_validation_layers)
		{
			createinfo.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
			createinfo.ppEnabledLayerNames = m_validation_layers.data();
			fill_debug_utils_messenger_create_info(messenger_info);
			createinfo.pNext = &messenger_info;
		}
		else
		{
			createinfo.enabledLayerCount = 0;
			createinfo.ppEnabledLayerNames = nullptr;
		}

		m_vulkan_instance = vk::createInstanceUnique(createinfo);

		setup_debug_messenger(m_vulkan_instance, messenger_info);
	}

	void Render::setup_debug_messenger(vk::UniqueInstance& instance, vk::DebugUtilsMessengerCreateInfoEXT& messenger_info)
	{
		// Vulkan hpp gives us a DispatchLoaderDynamic that basically loads the requiired method for us
		vk::DispatchLoaderDynamic dldi;
		dldi.init(*instance, vkGetInstanceProcAddr);
		auto messenger = instance->createDebugUtilsMessengerEXTUnique(messenger_info, nullptr, dldi);
	}

	void Render::create_surface(const Window& window, VkSurfaceKHR& surface)
	{
		if (glfwCreateWindowSurface(*m_vulkan_instance, window.get_raw_window(), nullptr, &surface) != VK_SUCCESS)
		{
			throw::std::runtime_error("failed to create window surface!");
		}
	}

}
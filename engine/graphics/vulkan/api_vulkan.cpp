#include "vulkan/api_vulkan.h"
#include "file_helper.h"
#include "vertex_data.h"

// TODO: Replace exceptions with logs (can also throw exceptions but they must be engine-breaking!)

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

namespace BE_NAMESPACE
{
	APIVulkan::APIVulkan(Window& window, bool enable_validation_layers)
		:b_use_validation_layers(enable_validation_layers),
		m_window_ref(window)
	{
		create_instance(window, enable_validation_layers);
		create_surface(window, m_surface);
		m_physical_device = select_physical_device();
		m_device = create_logical_device(m_physical_device);
		auto families = get_queue_families(m_physical_device);
		m_graphics_queue = m_device.getQueue(families.graphics.value(), 0);
		m_present_queue = m_device.getQueue(families.present.value(), 0);
		m_transfer_queue = m_device.getQueue(families.transfer.value(), 0);
		m_compute_queue = m_device.getQueue(families.compute.value(), 0);
		m_swapchain_info = create_swapchain(m_physical_device, m_surface, m_device);

		m_example_pipeline = create_example_pipeline();
	}

	APIVulkan::~APIVulkan()
	{
		m_device.destroyPipeline(m_example_pipeline);
		m_device.destroyRenderPass(m_example_renderpass);
		m_device.destroyPipelineLayout(m_example_layout);
		m_device.destroyDescriptorSetLayout(m_example_descriptor_set_layout);
		for (const auto& image_view : m_swapchain_info.image_views)
		{
			m_device.destroyImageView(image_view);
		}
		m_device.destroySwapchainKHR(m_swapchain_info.swapchain);
		m_device.destroy();
		vkDestroySurfaceKHR(*m_vulkan_instance, m_surface, nullptr);
	}

	void APIVulkan::draw_frame()
	{

	}

	void APIVulkan::create_instance(const Window& window, bool enable_validation_layers)
	{
		vk::ApplicationInfo appInfo(
			window.get_title().c_str(),
			vk::makeVersion(0, 1, 0),
			"Bomb Engine",
			vk::makeVersion(0, 1, 0),
			vk::ApiVersion13,
			nullptr
		);

		// not using constructor here to allow flexibility for the validation layers
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

		if (enable_validation_layers)
		{
			setup_debug_messenger(m_vulkan_instance, messenger_info);
		}
	}

	void APIVulkan::setup_debug_messenger(vk::UniqueInstance& instance, vk::DebugUtilsMessengerCreateInfoEXT& messenger_info)
	{
		// Vulkan hpp gives us a DispatchLoaderDynamic that basically loads the requiired method for us
		static vk::DispatchLoaderDynamic dldi;
		dldi.init(*instance, vkGetInstanceProcAddr);
		auto messenger = instance->createDebugUtilsMessengerEXTUnique(messenger_info, nullptr, dldi);
	}

	void APIVulkan::create_surface(const Window& window, VkSurfaceKHR& surface)
	{
		if (glfwCreateWindowSurface(*m_vulkan_instance, window.get_raw_window(), nullptr, &surface) != VK_SUCCESS)
		{
			throw::std::runtime_error("failed to create window surface!");
		}
	}

	vk::PhysicalDevice APIVulkan::select_physical_device()
	{
		auto physical_devices = m_vulkan_instance->enumeratePhysicalDevices();
		if (physical_devices.empty())
		{
			throw std::runtime_error("failed to find a supported GPU!");
		}

		std::map<uint32_t, vk::PhysicalDevice, std::greater<uint32_t>> device_ratings;

		for (const auto& p_device : physical_devices)
		{
			if (physical_device_is_suitable(p_device))
			{
				auto rating = rate_physical_device(p_device);
				device_ratings.emplace(rating, p_device);
			}
		}

		if (physical_devices.empty())
		{
			throw std::runtime_error("failed to find a supported GPU!");
		}

		// TODO: use logging macro instead
		std::cout << "Picked Device: " << device_ratings.begin()->second.getProperties().deviceName << std::endl;

		return device_ratings.begin()->second;
	}

	bool APIVulkan::physical_device_is_suitable(vk::PhysicalDevice physical_device)
	{
		auto families = get_queue_families(physical_device);
		if (!families.is_complete())
		{
			return false;
		}

		if (!check_extensions_support(physical_device))
		{
			return false;
		}

		auto swapchain_details = VkSwapchainDetails(physical_device, m_surface);
		if (swapchain_details.formats.empty() || swapchain_details.present_modes.empty())
		{
			return false;
		}

		return true;
	}

	uint32_t APIVulkan::rate_physical_device(vk::PhysicalDevice physical_device)
	{
		uint32_t score = 0;

		auto properties = physical_device.getProperties();
		auto memory_properties = physical_device.getMemoryProperties();
		auto features = physical_device.getFeatures();

		if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		{
			score += 1000;
		}

		score += properties.limits.maxComputeSharedMemorySize;
		score += properties.limits.maxImageDimension2D + properties.limits.maxImageDimension3D;

		for (const auto& heap : memory_properties.memoryHeaps)
		{
			if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
			{
				// heap size is in bytes, we want it in the order of Mb to keep it somewhat proportionate
				score += static_cast<uint32_t>(std::round(heap.size * 1e-6f));
			}
		}

		return score;
	}

	VkQueueFamilyIndices APIVulkan::get_queue_families(vk::PhysicalDevice physical_device)
	{
		VkQueueFamilyIndices families{};

		auto queue_families = physical_device.getQueueFamilyProperties();

		for (uint32_t family_index = 0; auto & queue_family : queue_families)
		{
			if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				families.graphics = family_index;
			}

			if (physical_device.getSurfaceSupportKHR(family_index, m_surface))
			{
				families.present = family_index;
			}

			if (!(queue_family.queueFlags & vk::QueueFlagBits::eGraphics) &&
				(queue_family.queueFlags & vk::QueueFlagBits::eTransfer)/* == vk::QueueFlagBits::eTransfer*/)
			{
				families.transfer = family_index;
			}

			if (!(queue_family.queueFlags & vk::QueueFlagBits::eGraphics) &&
				queue_family.queueFlags & vk::QueueFlagBits::eCompute)
			{
				families.compute = family_index;
			}

			if (families.is_complete())
			{
				break;
			}
		}

		return families;
	}

	bool APIVulkan::check_extensions_support(vk::PhysicalDevice physical_device)
	{
		auto available_extensions = physical_device.enumerateDeviceExtensionProperties();

		std::set<std::string> required_extensions{ m_required_device_extensions.begin(), m_required_device_extensions.end() };

		for (const auto& extension : available_extensions)
		{
			required_extensions.erase(extension.extensionName);
		}

		return required_extensions.empty();
	}

	vk::Device APIVulkan::create_logical_device(vk::PhysicalDevice physical_device)
	{
		auto families = get_queue_families(physical_device);

		std::set<uint32_t> unique_families = {
			families.graphics.value(),
			families.present.value(),
			families.transfer.value(),
			families.compute.value(),
		};

		// atm same priority for every queue, when we have a more complex rendering solution I might change it
		std::array<float, 4> priorities{ 1.0f };

		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos{};

		for (const auto& queue_family : unique_families)
		{
			vk::DeviceQueueCreateInfo queue_create_info(vk::DeviceQueueCreateFlags(), queue_family, priorities);
			queue_create_infos.push_back(queue_create_info);
		}

		// TODO: have a better features selection
		vk::PhysicalDeviceFeatures device_features{};
		device_features.samplerAnisotropy = vk::True;
		device_features.sampleRateShading = vk::True;

		vk::DeviceCreateInfo create_info(
			vk::DeviceCreateFlags(),
			queue_create_infos,
			nullptr,
			m_required_device_extensions,
			&device_features);

		if (b_use_validation_layers)
		{
			create_info.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
			create_info.ppEnabledLayerNames = m_validation_layers.data();
		}

		return physical_device.createDevice(create_info);
	}

	VkSwapchainInfo APIVulkan::create_swapchain(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, vk::Device device, vk::SwapchainKHR old_swapchain)
	{
		auto swapchain_details = VkSwapchainDetails(physical_device, surface);
		auto [format, color_space] = choose_swapchain_surface_format(swapchain_details.formats);
		auto present_mode = choose_swapchain_present_mode(swapchain_details.present_modes);
		auto extent = choose_swapchain_extent(swapchain_details.capabilities);

		uint32_t image_count = swapchain_details.capabilities.minImageCount + 1;
		if (swapchain_details.capabilities.maxImageCount > 0 &&
			image_count > swapchain_details.capabilities.maxImageCount)
		{
			image_count = swapchain_details.capabilities.maxImageCount;
		}

		auto indices = get_queue_families(physical_device);
		std::vector<uint32_t> swapchain_sharing_indicies{};
		uint32_t sharing_index_count = 0;
		auto sharing_mode = vk::SharingMode::eExclusive;
		if (indices.graphics != indices.present)
		{
			sharing_index_count = 2;
			swapchain_sharing_indicies = { indices.graphics.value(), indices.present.value() };
			sharing_mode = vk::SharingMode::eConcurrent;
		}

		vk::SwapchainCreateInfoKHR create_info(vk::SwapchainCreateFlagsKHR(),
			surface,
			image_count,
			format,
			color_space,
			extent,
			1,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment,
			sharing_mode,
			sharing_index_count,
			swapchain_sharing_indicies.data(),
			swapchain_details.capabilities.currentTransform,
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			present_mode,
			true,
			old_swapchain,
			nullptr);

		VkSwapchainInfo swapchain_info{};
		swapchain_info.swapchain = device.createSwapchainKHR(create_info);
		swapchain_info.extent = extent;
		swapchain_info.format = format;
		swapchain_info.images = device.getSwapchainImagesKHR(swapchain_info.swapchain);

		swapchain_info.image_views.reserve(swapchain_info.images.size());
		vk::ImageViewCreateInfo image_view_create_info(
			vk::ImageViewCreateFlags(),
			{},
			vk::ImageViewType::e2D,
			format,
			{},
			vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

		for (const auto& image : swapchain_info.images)
		{
			image_view_create_info.image = image;
			swapchain_info.image_views.push_back(device.createImageView(image_view_create_info));
		}

		return  swapchain_info;
	}

	vk::SurfaceFormatKHR APIVulkan::choose_swapchain_surface_format(std::vector<vk::SurfaceFormatKHR> formats)
	{
		for (const auto& format : formats)
		{
			if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return format;
			}
		}
		return formats[0];
	}

	vk::PresentModeKHR APIVulkan::choose_swapchain_present_mode(std::vector<vk::PresentModeKHR> present_modes)
	{
		for (const auto& present_mode : present_modes)
		{
			if (present_mode == vk::PresentModeKHR::eMailbox) // to render frames as fast as possible
			{
				return present_mode;
			}
		}
		// guaranteed by spec
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D APIVulkan::choose_swapchain_extent(vk::SurfaceCapabilitiesKHR capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		int width, height;
		glfwGetFramebufferSize(m_window_ref.get_raw_window(), &width, &height);

		vk::Extent2D extent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

		return std::clamp(extent, capabilities.minImageExtent, capabilities.maxImageExtent);
	}

	vk::ShaderModule APIVulkan::create_shader_module(SPIRVShader& shader)
	{
		vk::ShaderModuleCreateInfo create_info(vk::ShaderModuleCreateFlagBits(),
			shader.get_bytes_count(),
			shader.get_data().data()
		);
		return m_device.createShaderModule(create_info);
	}						

	vk::Pipeline APIVulkan::create_example_pipeline()
	{
		SPIRVShader vertex, fragment;
		if (auto vertex_file = file_helper::load_file("shaders/vertex.spv"); vertex_file.has_value())
		{
			vertex = SPIRVShader(vertex_file.value());
		}
		else
		{
			throw std::runtime_error("error parsing file");
		}
		if (auto fragment_file = file_helper::load_file("shaders/fragment.spv"); fragment_file.has_value())
		{
			fragment = SPIRVShader(fragment_file.value());
		}
		else
		{
			throw std::runtime_error("error parsing file");
		}
		auto vertex_shader = create_shader_module(vertex);
		auto fragment_shader = create_shader_module(fragment);

		vk::PipelineShaderStageCreateInfo vert_stage(vk::PipelineShaderStageCreateFlagBits(), vk::ShaderStageFlagBits::eVertex, vertex_shader, "main");
		vk::PipelineShaderStageCreateInfo frag_stage(vk::PipelineShaderStageCreateFlagBits(), vk::ShaderStageFlagBits::eFragment, fragment_shader, "main");

		std::array<vk::PipelineShaderStageCreateInfo, 2> stages{ vert_stage, frag_stage };

		vk::PipelineDepthStencilStateCreateInfo depth_stencil(vk::PipelineDepthStencilStateCreateFlagBits(),
			true,
			true,
			vk::CompareOp::eLess);

		auto binding_desc = VertexData::get_binding_description();
		auto attrib_desc = VertexData::get_attribute_descriptions();
		vk::PipelineVertexInputStateCreateInfo vert_input(vk::PipelineVertexInputStateCreateFlagBits(), binding_desc, attrib_desc);

		std::array<vk::DynamicState, 2> dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		vk::PipelineDynamicStateCreateInfo dynamic_state_info(vk::PipelineDynamicStateCreateFlagBits(), dynamic_states);

		vk::Viewport viewport(0, 0, m_swapchain_info.extent.width, m_swapchain_info.extent.height, 0, 1);
		vk::Rect2D scissor({ 0,0 }, m_swapchain_info.extent);
		vk::PipelineViewportStateCreateInfo viewport_state(vk::PipelineViewportStateCreateFlagBits(), viewport, scissor);

		vk::PipelineInputAssemblyStateCreateInfo input_assembly(vk::PipelineInputAssemblyStateCreateFlagBits(), vk::PrimitiveTopology::eTriangleList, false);

		vk::PipelineRasterizationStateCreateInfo raster(vk::PipelineRasterizationStateCreateFlagBits(), false, false); // default is ok
		raster.lineWidth = 1.0f;	// it was complaining, probably default is 0

		vk::PipelineMultisampleStateCreateInfo multisampling(vk::PipelineMultisampleStateCreateFlagBits(), vk::SampleCountFlagBits::e8);

		vk::PipelineColorBlendAttachmentState color_attachment(
			true,
			vk::BlendFactor::eSrcAlpha,
			vk::BlendFactor::eOneMinusSrcAlpha, 
			vk::BlendOp::eAdd, 
			vk::BlendFactor::eOne, 
			vk::BlendFactor::eZero, 
			vk::BlendOp::eAdd, 
			vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

		vk::PipelineColorBlendStateCreateInfo color_blend(vk::PipelineColorBlendStateCreateFlagBits(), false, vk::LogicOp::eClear, color_attachment);

		m_example_layout = create_example_pipeline_layout();

		m_example_renderpass = create_example_render_pass();

		vk::GraphicsPipelineCreateInfo pipeline(vk::PipelineCreateFlagBits(),
			stages,
			&vert_input,
			&input_assembly,
			nullptr,
			&viewport_state,
			&raster,
			&multisampling,
			&depth_stencil,
			&color_blend,
			&dynamic_state_info,
			m_example_layout,
			m_example_renderpass,
			0
		);

		auto pipeline_res = m_device.createGraphicsPipeline(nullptr, pipeline);
		if (pipeline_res.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		m_device.destroyShaderModule(vertex_shader);
		m_device.destroyShaderModule(fragment_shader);

		return pipeline_res.value;
	}
	vk::RenderPass APIVulkan::create_example_render_pass()
	{
		vk::AttachmentDescription color_attachment(vk::AttachmentDescriptionFlagBits(),
			m_swapchain_info.format,
			vk::SampleCountFlagBits::e8,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal
		);

		vk::AttachmentDescription depth_attachment(vk::AttachmentDescriptionFlagBits(),
			vk::Format::eD32SfloatS8Uint,
			vk::SampleCountFlagBits::e8,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal);

		vk::AttachmentDescription color_attachment_resolve(vk::AttachmentDescriptionFlagBits(),
			m_swapchain_info.format,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR);

		vk::AttachmentReference color_ref(0, vk::ImageLayout::eColorAttachmentOptimal);
		vk::AttachmentReference depth_stencil_ref(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
		vk::AttachmentReference color_resolve_ref(2, vk::ImageLayout::eColorAttachmentOptimal);

		vk::SubpassDescription subpass(vk::SubpassDescriptionFlagBits(),
			vk::PipelineBindPoint::eGraphics
		);
		subpass.setColorAttachments(color_ref);
		subpass.setPDepthStencilAttachment(&depth_stencil_ref);
		subpass.setResolveAttachments(color_resolve_ref);


		vk::SubpassDependency dependency(vk::SubpassExternal,
			0,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
			vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite
		);

		std::array<vk::AttachmentDescription, 3> attachments { color_attachment, depth_attachment, color_attachment_resolve };
		vk::RenderPassCreateInfo render_pass_info(vk::RenderPassCreateFlagBits(),
			attachments,
			subpass,
			dependency);

		return m_device.createRenderPass(render_pass_info);
	}
	vk::PipelineLayout APIVulkan::create_example_pipeline_layout()
	{
		vk::DescriptorSetLayoutBinding projection(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
		vk::DescriptorSetLayoutBinding sampler(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);

		std::array<vk::DescriptorSetLayoutBinding, 2> bindings{ projection, sampler };
		
		vk::DescriptorSetLayoutCreateInfo layout_info(vk::DescriptorSetLayoutCreateFlagBits(), bindings);
		m_example_descriptor_set_layout = m_device.createDescriptorSetLayout(layout_info);

		vk::PipelineLayoutCreateInfo pipeline_layout(vk::PipelineLayoutCreateFlagBits(), m_example_descriptor_set_layout);

		return m_device.createPipelineLayout(pipeline_layout);
	}
}